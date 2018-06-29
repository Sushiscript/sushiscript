#include "sushi/type-system/type-check/statement.h"
#include "sushi/type-system/type-check/expression.h"
#include <algorithm>

#define SIMPLE(t) BuiltInAtom::Type::t
#define MAKE_SIMPLE(t) type::BuiltInAtom::Make(SIMPLE(t))

namespace sushi {

namespace type {

namespace {

Type::Pointer FromTypeExpr(const ast::TypeExpr *expr);

struct ToType : ast::TypeExprVisitor::Const {
    SUSHI_VISITING(ast::TypeLit, t) {
        res = BuiltInAtom::Make(t.type);
    }
    SUSHI_VISITING(ast::ArrayType, t) {
        res = Array::Make(t.element);
    }
    SUSHI_VISITING(ast::MapType, t) {
        res = Map::Make(t.key, t.value);
    }
    SUSHI_VISITING(ast::FunctionType, t) {
        Type::Pointer ret = FromTypeExpr(t.result.get());
        std::vector<Type::Pointer> params(t.params.size());
        std::transform(
            begin(t.params), end(t.params), begin(params),
            [](auto &p) { return FromTypeExpr(p.get()); });
        res = Function::Make(std::move(params), std::move(ret));
    }
    Type::Pointer res;
};

Type::Pointer FromTypeExpr(const ast::TypeExpr *expr) {
    ToType to_type;
    expr->AcceptVisitor(to_type);
    return std::move(to_type.res);
}

bool IsAssignable(const ast::Expression *expr) {
    if (auto idx = dynamic_cast<const ast::Indexing *>(expr)) {
        return IsAssignable(idx->indexable.get());
    }
    auto var = dynamic_cast<const ast::Variable *>(expr);
    return var != nullptr;
}

struct CheckStatementVisitor : ast::StatementVisitor::Const {
    CheckStatementVisitor(State &s) : s(s) {}

    SUSHI_VISITING(ast::Assignment, ass) {
        if (not IsAssignable(ass.lvalue.get())) {
            s.TypeError(ass.lvalue.get(), Error::kInvalidLvalue);
            return;
        }
        auto left_type = UnambiguousDeduce(*ass.lvalue, s);
        SatisfyRequirement(*ass.value, std::move(left_type), s);
    }

    SUSHI_VISITING(ast::VariableDef, vdef) {
        Type::Pointer var_type;
        if (not vdef.type) {
            var_type = UnambiguousDeduce(*vdef.value, s);
        } else {
            auto should_be = FromTypeExpr(vdef.type.get());
            if (SatisfyRequirement(*vdef.value, should_be->Copy(), s)) {
                var_type = std::move(should_be);
            }
        }
        s.InsertName(vdef.name, std::move(var_type));
    }
    std::vector<Type::Pointer> ExtractParamTypes(const ast::FunctionDef &fdef) {
        auto &params = fdef.params;
        std::vector<Type::Pointer> param_types;
        auto fbody = s.FromNewProgram(fdef.body);
        for (auto &p : params) {
            auto t = FromTypeExpr(p.type.get());
            fbody.InsertName(p.name, t->Copy());
            param_types.push_back(std::move(t));
        }
        if (param_types.empty()) {
            param_types.push_back(MAKE_SIMPLE(kUnit));
        }
        return param_types;
    }
    void DeclaredReturnType(
        const ast::FunctionDef &fdef, std::vector<Type::Pointer> params) {
        auto ret_type = FromTypeExpr(fdef.ret_type.get());
        s.InsertName(
            fdef.name, Function::Make(std::move(params), ret_type->Copy()));
        CheckProgram(s.NewFunctionBody(fdef.body, ret_type->Copy()));
    }
    void DeducingReturnType(
        const ast::FunctionDef &fdef, std::vector<Type::Pointer> params) {
        State new_state = s.NewFunctionBody(fdef.body, nullptr);
        CheckProgram(new_state);
        Type::Pointer ret = s.return_type == nullptr ? MAKE_SIMPLE(kUnit)
                                                     : std::move(s.return_type);
        s.InsertName(
            fdef.name, Function::Make(std::move(params), std::move(ret)));
    }
    SUSHI_VISITING(ast::FunctionDef, fdef) {
        auto params = ExtractParamTypes(fdef);
        if (fdef.ret_type) {
            DeclaredReturnType(fdef, std::move(params));
        } else {
            DeducingReturnType(fdef, std::move(params));
        }
    }

    SUSHI_VISITING(ast::IfStmt, if_) {
        SatisfyRequirement(*if_.condition, MAKE_SIMPLE(kBool), s);
        CheckProgram(s.FromNewProgram(if_.true_body));
        CheckProgram(s.FromNewProgram(if_.false_body));
    }

    void NoReturnValue() {
        if (not s.return_type) {
            s.return_type = MAKE_SIMPLE(kUnit);
        } else if (not s.return_type->Equals(MAKE_SIMPLE(kUnit))) {
            s.TypeError(nullptr, Error::MissingReturnValue);
        }
    }

    SUSHI_VISITING(ast::ReturnStmt, ret) {
        if (ret.value == nullptr) {
            NoReturnValue();
        } else if (s.return_type == nullptr) {
            s.return_value = UnambiguousDeduce(*ret.value, s);
        } else {
            SatisfyRequirement(*ret.value, s.return_type->Copy(), s);
        }
    }

    SUSHI_VISITING(ast::SwitchStmt, sw) {
        auto tp = UnambiguousDeduce(*sw.switched, s);
        for (auto &c : sw.cases) {
            if (c.condition and
                not SatisfyRequirement(*c.condition, tp->Copy(), s)) {
                continue;
            }
            CheckProgram(s.FromNewProgram(c.body));
        }
    }

    Type::Pointer GetIdentType(const ast::ForStmt::Condition &cond) {
        auto range_type = UnambiguousDeduce(*cond.condition, s, false);
        if (not range_type) return nullptr;
        auto arr = range_type->ToArray();
        if (not arr)
            return s.TypeError(cond.condition.get(), Error::kInvalidRange);
        auto elem = arr->element;
        return BuiltInAtom::Make(elem);
    }

    SUSHI_VISITING(ast::ForStmt, for_) {
        if (for_.condition.IsRange()) {
            auto tp = GetIdentType(for_.condition);
            if (not tp) return;
            s.FromNewProgram(for_.body).InsertName(
                for_.condition.ident_name, std::move(tp));
        } else {
            SatisfyRequirement(
                *for_.condition.condition, MAKE_SIMPLE(kBool), s);
        }
        CheckProgram(s.FromNewProgram(for_.body));
    }

    SUSHI_VISITING(ast::LoopControlStmt, l) {
        // NOP
    }

    SUSHI_VISITING(ast::Expression, expr) {
        UnambiguousDeduce(expr, s);
    }
    State &s;
};

void CheckStatement(const ast::Statement &stmt, State &state) {
    CheckStatementVisitor v(state);
    stmt.AcceptVisitor(stmt);
}

} // namespace

void CheckProgram(State &&state) {
    auto &program = state.program;
    for (auto &stmt : program.statements) {
        CheckStatement(*stmt, state);
    }
}

void CheckProgram(State &state) {
    auto &program = state.program;
    for (auto &stmt : program.statements) {
        CheckStatement(*stmt, state);
    }
}

} // namespace type

} // namespace sushi