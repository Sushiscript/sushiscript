#include "sushi/type-system/type-check/expression.h"
#include "boost/optional.hpp"

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

bool ValidInterpolatedString(const ast::InterpolatedString &inter, State &s) {
    return true;
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

    SUSHI_VISITING(ast::UnaryExpr, u) {

    }

    SUSHI_VISITING(ast::BinaryExpr, bi) {

    }

    SUSHI_VISITING(ast::CommandLike, cmd) {

    }

    SUSHI_VISITING(ast::Indexing, idx) {

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

bool SatisfyRequirement(
    const ast::Expression &expr, Type::Pointer should_be, State &state) {
    auto result = Deduce(expr, state);
    if (result.fail) return false;
    if (result.empty_array and (should_be->ToMap() or should_be->ToArray())) {
        state.env.Insert(&expr, std::move(should_be));
        return true;
    }
    if (ImplicitConvertible(result.type.get(), should_be.get())) {
        state.env.Insert(&expr, std::move(result.type));
        return true;
    }
    state.TypeError(expr, Error::kInvalidType);
    return false;
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