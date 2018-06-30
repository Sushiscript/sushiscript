#include "sushi/type-system/type-check/expression.h"
#include "boost/optional.hpp"
#include "sushi/util/container.h"
#include <functional>

#define SIMPLE(t) BuiltInAtom::Type::t
#define MAKE_SIMPLE(t) type::BuiltInAtom::Make(SIMPLE(t))
#define V_RETURN(r)                                                            \
    {                                                                          \
        result = r;                                                            \
        return;                                                                \
    }

using boost::none;
using boost::optional;

namespace sushi {

namespace type {

namespace {

std::vector<Type::Pointer> Interpolatable() {
    return {MAKE_SIMPLE(kChar), MAKE_SIMPLE(kInt), MAKE_SIMPLE(kString),
            MAKE_SIMPLE(kPath), MAKE_SIMPLE(kRelPath)};
}

bool ValidInterpolatedString(const ast::InterpolatedString &inter, State &s) {
    bool success = true;
    inter.Traverse(
        [](auto &) {},
        [&success, &s](auto &expr) {
            auto t = RequireOneOf(expr, s, Interpolatable());
            success = t != nullptr and success;
        });
    return success;
}

DeduceResult RequireThen(
    const ast::Expression &expr, Type::Pointer should_be, State &state,
    Type::Pointer result) {
    if (SatisfyRequirement(expr, std::move(should_be), state))
        return DeduceResult(std::move(result));
    return DeduceResult::Fail();
}

optional<BuiltInAtom::Type>
DeduceSimple(const ast::Expression &expr, State &s) {
    auto t = UnambiguousDeduce(expr, s);
    if (not t) return none;
    auto st = t->ToSimple();
    if (not st) {
        s.TypeError(&expr, Error::kRequireSimpleType);
        return none;
    }
    return st->type;
}

struct DeduceCommandLikeVisitor : ast::CommandLikeVisitor::Const {
    DeduceCommandLikeVisitor(State &s) : s(s) {}

    bool CompatibleParams(const ast::FunctionCall &call, const Function *f) {
        auto &exprs = call.parameters;
        auto &types = f->params;
        if (exprs.size() != types.size()) {
            s.TypeError(&call, Error::kWrongNumOfParams);
            return false;
        }
        bool success = true;
        for (int i = 0; i < types.size(); ++i) {
            auto b = SatisfyRequirement(*exprs[i], types[i]->Copy(), s);
            success = success and b;
        }
        return success;
    }

    SUSHI_VISITING(ast::FunctionCall, fc) {
        auto tp = s.LookupName(fc.func);
        if (not tp) V_RETURN(nullptr);
        auto func_type = tp->ToFunction();
        if (not func_type) {
            s.TypeError(nullptr, Error::kInvalidFunction);
        } else if (CompatibleParams(fc, func_type)) {
            V_RETURN(func_type->result->Copy());
        }
        V_RETURN(nullptr);
    }

    SUSHI_VISITING(ast::Command, cmd) {
        bool success = ValidInterpolatedString(cmd.cmd, s);
        for (auto &p : cmd.parameters)
            success = ValidInterpolatedString(p, s) and success;
        V_RETURN(success ? MAKE_SIMPLE(kExitCode) : Type::Pointer{});
    }
    State &s;
    Type::Pointer result;
};

Type::Pointer DeduceCommandLike(const ast::CommandLike &cl, State &state) {
    DeduceCommandLikeVisitor v(state);
    cl.AcceptVisitor(v);
    return std::move(v.result);
}

struct DeduceLiteralVisitor : ast::LiteralVisitor::Const {
    DeduceLiteralVisitor(State &s) : s(s) {}

    SUSHI_VISITING(ast::IntLit, i) {
        V_RETURN(DeduceResult(MAKE_SIMPLE(kInt)));
    }

    SUSHI_VISITING(ast::CharLit, c) {
        V_RETURN(DeduceResult(MAKE_SIMPLE(kChar)));
    }

    SUSHI_VISITING(ast::BoolLit, b) {
        V_RETURN(DeduceResult(MAKE_SIMPLE(kBool)));
    }

    SUSHI_VISITING(ast::UnitLit, u) {
        V_RETURN(DeduceResult(MAKE_SIMPLE(kUnit)));
    }

    SUSHI_VISITING(ast::FdLit, f) {
        V_RETURN(DeduceResult(MAKE_SIMPLE(kFd)));
    }

    SUSHI_VISITING(ast::PathLit, p) {
        if (ValidInterpolatedString(p.value, s))
            V_RETURN(DeduceResult(MAKE_SIMPLE(kPath)));
        V_RETURN(DeduceResult::Fail());
    }

    SUSHI_VISITING(ast::RelPathLit, r) {
        if (ValidInterpolatedString(r.value, s))
            V_RETURN(DeduceResult(MAKE_SIMPLE(kRelPath)));
        V_RETURN(DeduceResult::Fail());
    }

    SUSHI_VISITING(ast::StringLit, str) {
        if (ValidInterpolatedString(str.value, s))
            V_RETURN(DeduceResult(MAKE_SIMPLE(kString)));
        V_RETURN(DeduceResult::Fail());
    }

    SUSHI_VISITING(ast::ArrayLit, a) {
        if (a.value.empty()) V_RETURN(DeduceResult::EmptyArray());
        auto elem = DeduceSimple(*a.value.front(), s);
        if (not elem) V_RETURN(DeduceResult::Fail());
        bool success = true;
        for (int i = 1; i < a.value.size(); ++i) {
            success =
                SatisfyRequirement(*a.value[i], BuiltInAtom::Make(*elem), s) and
                success;
        }
        V_RETURN(success ? Array::Make(*elem) : DeduceResult::Fail());
    }

    SUSHI_VISITING(ast::MapLit, m) {
        if (m.value.empty()) V_RETURN(DeduceResult::EmptyArray());
        auto key = DeduceSimple(*m.value.front().first, s);
        auto val = DeduceSimple(*m.value.front().second, s);
        if (not key or not val) V_RETURN(DeduceResult::Fail());
        bool success = true;
        for (int i = 1; i < m.value.size(); ++i) {
            auto &key_expr = *m.value[i].first;
            auto &val_expr = *m.value[i].second;
            auto ks = SatisfyRequirement(key_expr, BuiltInAtom::Make(*key), s),
                 vs = SatisfyRequirement(val_expr, BuiltInAtom::Make(*val), s);
            success = success and ks and vs;
        }
        V_RETURN(success ? Map::Make(*key, *val) : DeduceResult::Fail());
    }

    DeduceResult result;
    State &s;
};

DeduceResult DeduceLiteral(const ast::Literal &l, State &s) {
    DeduceLiteralVisitor v(s);
    l.AcceptVisitor(v);
    return std::move(v.result);
}

struct DeductionVisitor : ast::ExpressionVisitor::Const {
    DeductionVisitor(State &s) : s(s) {}

    SUSHI_VISITING(ast::Variable, var) {
        auto p = s.LookupName(var.var);
        if (not p) V_RETURN(DeduceResult::Fail());
        V_RETURN(DeduceResult(std::move(p)));
    }

    SUSHI_VISITING(ast::Literal, lit) {
        V_RETURN(DeduceLiteral(lit, s));
    }

    DeduceResult Negate(const ast::Expression &expr) {
        if (SatisfyRequirement(expr, MAKE_SIMPLE(kBool), s))
            return DeduceResult(MAKE_SIMPLE(kBool));
        return DeduceResult::Fail();
    }

    DeduceResult PosNeg(const ast::Expression &expr) {
        if (SatisfyRequirement(expr, MAKE_SIMPLE(kInt), s))
            return DeduceResult(MAKE_SIMPLE(kInt));
        return DeduceResult::Fail();
    }

    SUSHI_VISITING(ast::UnaryExpr, u) {
        using UO = ast::UnaryExpr::Operator;
        switch (u.op) {
        case UO::kNot: V_RETURN(Negate(*u.expr));
        case UO::kNeg:
        case UO::kPos: V_RETURN(PosNeg(*u.expr));
        }
    }

    static Type::Pointer Addable(const DeduceResult &d) {
        if (d.type == nullptr) return nullptr;
        if (d.type->ToMap() or d.type->ToArray()) return d.type->Copy();
        if (auto simple = d.type->ToSimple()) {
            if (simple->type == SIMPLE(kInt) or simple->type == SIMPLE(kString))
                return d.type->Copy();
        }
        return nullptr;
    }

    DeduceResult Add(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto tp = RequireSatisfy(lhs, s, &Addable);
        if (tp != nullptr and SatisfyRequirement(rhs, tp->Copy(), s)) {
            return DeduceResult(std::move(tp));
        }
        return DeduceResult::Fail();
    }

    DeduceResult Minus(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto int_ = MAKE_SIMPLE(kInt);
        if (SatisfyRequirement(lhs, int_->Copy(), s) and
            SatisfyRequirement(rhs, int_->Copy(), s))
            return DeduceResult(std::move(int_));
        return DeduceResult::Fail();
    }

    static std::vector<Type::Pointer> Multipliable() {
        return {MAKE_SIMPLE(kInt), MAKE_SIMPLE(kPath)};
    }

    DeduceResult Mult(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto left_type = RequireOneOf(lhs, s, Multipliable());
        if (left_type and SatisfyRequirement(rhs, MAKE_SIMPLE(kInt), s)) {
            return DeduceResult(std::move(left_type));
        }
        return DeduceResult::Fail();
    }

    static std::vector<Type::Pointer> Dividable() {
        return {MAKE_SIMPLE(kInt), MAKE_SIMPLE(kPath), MAKE_SIMPLE(kRelPath)};
    }

    DeduceResult Div(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto tp = RequireOneOf(lhs, s, Dividable());
        if (not tp) return DeduceResult::Fail();
        auto int_ = MAKE_SIMPLE(kInt);
        if (tp->Equals(int_->Copy())) {
            return RequireThen(rhs, int_->Copy(), s, int_->Copy());
        } else {
            return RequireThen(rhs, MAKE_SIMPLE(kRelPath), s, std::move(tp));
        }
        return DeduceResult::Fail();
    }

    DeduceResult Mod(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto int_ = MAKE_SIMPLE(kInt);
        if (SatisfyRequirement(lhs, int_->Copy(), s) and
            SatisfyRequirement(rhs, int_->Copy(), s))
            return DeduceResult(std::move(int_));
        return DeduceResult::Fail();
    }

    static bool IsEqualComparable(BuiltInAtom::Type t) {
        std::vector<BuiltInAtom::Type> v{
            SIMPLE(kUnit),   SIMPLE(kBool), SIMPLE(kChar),     SIMPLE(kInt),
            SIMPLE(kString), SIMPLE(kPath), SIMPLE(kExitCode), SIMPLE(kFd)};
        return sushi::util::Has(v, t);
    }

    static bool IsEqualComparable(const Type *tp) {
        if (auto arr = tp->ToArray()) {
            return IsEqualComparable(arr->element);
        }
        if (auto s = tp->ToSimple()) {
            return IsEqualComparable(s->type);
        }

        return false;
    }

    static Type::Pointer EqualComparable(const DeduceResult &d) {
        if (d.type == nullptr) return nullptr;
        if (IsEqualComparable(d.type.get())) return d.type->Copy();
        return nullptr;
    }

    DeduceResult Equal(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto tp = RequireSatisfy(lhs, s, &EqualComparable);
        if (not tp) return DeduceResult::Fail();
        return RequireThen(rhs, tp->Copy(), s, MAKE_SIMPLE(kBool));
    }

    DeduceResult Logic(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto boo = MAKE_SIMPLE(kBool);
        if (SatisfyRequirement(lhs, boo->Copy(), s) and
            SatisfyRequirement(rhs, boo->Copy(), s))
            return DeduceResult(std::move(boo));
        return DeduceResult::Fail();
    }

    static std::vector<Type::Pointer> OrderComparable() {
        return {MAKE_SIMPLE(kChar), MAKE_SIMPLE(kInt), MAKE_SIMPLE(kString)};
    }

    DeduceResult Order(const ast::Expression &lhs, const ast::Expression &rhs) {
        auto tp = RequireOneOf(lhs, s, OrderComparable());
        if (not tp) return DeduceResult::Fail();
        return RequireThen(rhs, std::move(tp), s, MAKE_SIMPLE(kBool));
    }

    SUSHI_VISITING(ast::BinaryExpr, bi) {
        using BO = ast::BinaryExpr::Operator;
        switch (bi.op) {
        case BO::kAdd: V_RETURN(Add(*bi.lhs, *bi.rhs));
        case BO::kMinus: V_RETURN(Minus(*bi.lhs, *bi.rhs));
        case BO::kMult: V_RETURN(Mult(*bi.lhs, *bi.rhs));
        case BO::kDiv: V_RETURN(Div(*bi.lhs, *bi.rhs));
        case BO::kMod: V_RETURN(Mod(*bi.lhs, *bi.rhs));
        case BO::kLess:
        case BO::kGreat:
        case BO::kLessEq:
        case BO::kGreatEq: V_RETURN(Order(*bi.lhs, *bi.rhs));
        case BO::kEqual:
        case BO::kNotEq: V_RETURN(Equal(*bi.lhs, *bi.rhs));
        case BO::kAnd:
        case BO::kOr: V_RETURN(Logic(*bi.lhs, *bi.rhs));
        }
    }

    static std::vector<Type::Pointer> FromRedirAccepts() {
        return {MAKE_SIMPLE(kPath), MAKE_SIMPLE(kRelPath)};
    }
    static std::vector<Type::Pointer> ToRedirAccepts() {
        return {MAKE_SIMPLE(kPath), MAKE_SIMPLE(kRelPath), MAKE_SIMPLE(kFd)};
    }

    bool
    ValidRedirections(const std::vector<ast::Redirection> &redirs, bool &here) {
        bool success = false;
        for (auto &redir : redirs) {
            if (not redir.external) {
                here = true;
                continue;
            };
            auto accepts = redir.direction == ast::Redirection::Direction::kIn
                               ? FromRedirAccepts()
                               : ToRedirAccepts();
            auto b = bool(RequireOneOf(*redir.external, s, std::move(accepts)));
            success = success and b;
        }
        return success;
    }

    DeduceResult CheckCommandLike(bool success, const ast::CommandLike &cmd) {
        bool here = false;
        auto valid_redirs = ValidRedirections(cmd.redirs, here);
        auto type = DeduceCommandLike(cmd, s);
        success = valid_redirs and type != nullptr and success;
        if (cmd.pipe_next != nullptr)
            return CheckCommandLike(success, *cmd.pipe_next);
        return success ? (here ? DeduceResult(MAKE_SIMPLE(kString))
                               : DeduceResult(std::move(type)))
                       : DeduceResult::Fail();
    }

    SUSHI_VISITING(ast::CommandLike, cmd) {
        V_RETURN(CheckCommandLike(true, cmd));
    }

    SUSHI_VISITING(ast::Indexing, idx) {
        auto tp = UnambiguousDeduce(*idx.indexable, s);
        if (not tp) V_RETURN(DeduceResult::Fail());
        if (auto a = tp->ToArray())
            V_RETURN(RequireThen(
                *idx.index, MAKE_SIMPLE(kInt), s,
                BuiltInAtom::Make(a->element)));
        if (auto m = tp->ToMap())
            V_RETURN(RequireThen(
                *idx.index, BuiltInAtom::Make(m->key), s,
                BuiltInAtom::Make(m->value)));
        if (tp->Equals(MAKE_SIMPLE(kString)))
            V_RETURN(RequireThen(
                *idx.index, MAKE_SIMPLE(kInt), s, MAKE_SIMPLE(kChar)));

        s.TypeError(idx.indexable.get(), Error::kInvalidIndexable);
        V_RETURN(DeduceResult::Fail());
    }

    State &s;
    DeduceResult result;
};

bool ImplicitConvertible(const Type *from, const Type *to) {
    if (from->Equals(to)) return true;
    auto simple_from = from->ToSimple(), simple_to = to->ToSimple();
    if (not(simple_from and simple_to)) return false;
    auto from_type = simple_from->type, to_type = simple_to->type;
    if (from_type == to_type) return true;
    if (from_type == SIMPLE(kExitCode) and
        (to_type == SIMPLE(kBool) or to_type == SIMPLE(kInt)))
        return true;
    if (from_type == SIMPLE(kRelPath) and to_type == SIMPLE(kPath)) return true;
    return false;
}

} // namespace

DeduceResult Deduce(const ast::Expression &expr, State &state) {
    DeductionVisitor v(state);
    expr.AcceptVisitor(v);
    return std::move(v.result);
}

Type::Pointer SolveRequirement(const DeduceResult &d, Type::Pointer should_be) {
    if (d.fail) return nullptr;
    if (d.empty_array and (should_be->ToMap() or should_be->ToArray())) {
        return should_be;
    }
    if (ImplicitConvertible(d.type.get(), should_be.get())) {
        return d.type->Copy();
    }
    return nullptr;
}

bool SatisfyRequirement(
    const ast::Expression &expr, Type::Pointer should_be, State &state) {
    auto tp = RequireSatisfy(expr, state, [&should_be](const DeduceResult &d) {
        return SolveRequirement(d, std::move(should_be));
    });
    return tp != nullptr;
}

Type::Pointer RequireOneOf(
    const ast::Expression &expr, State &state, std::vector<Type::Pointer> ts) {
    auto result = Deduce(expr, state);
    if (result.fail) return nullptr;
    Type::Pointer result_type;
    for (auto &tp : ts) {
        auto t = SolveRequirement(result, std::move(tp));
        if (not t) continue;
        if (result_type != nullptr)
            return state.TypeError(&expr, Error::kAmbiguousType);
        result_type = std::move(t);
    }
    if (result_type == nullptr) {
        return state.TypeError(&expr, Error::kInvalidType);
    }
    state.env.Insert(&expr, result_type->Copy());
    return result_type;
}

Type::Pointer RequireSatisfy(
    const ast::Expression &expr, State &state,
    std::function<Type::Pointer(const DeduceResult &)> pred) {
    auto result = Deduce(expr, state);
    if (result.fail) return nullptr;
    Type::Pointer result_type = pred(result);
    if (result_type == nullptr) {
        return state.TypeError(&expr, Error::kInvalidType);
    }
    state.env.Insert(&expr, result_type->Copy());
    return result_type;
}

Type::Pointer
UnambiguousDeduce(const ast::Expression &expr, State &s, bool insert) {
    auto deduced = Deduce(expr, s);
    if (deduced.fail) return nullptr;
    if (not deduced.type) {
        s.TypeError(&expr, Error::kAmbiguousType);
        return nullptr;
    }
    if (insert) {
        s.env.Insert(&expr, deduced.type->Copy());
    }
    return std::move(deduced.type);
}

} // namespace type

} // namespace sushi