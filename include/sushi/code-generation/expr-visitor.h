#ifndef SUSHI_CODE_GEN_EXPR_VISITOR_H_
#define SUSHI_CODE_GEN_EXPR_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope.h"
#include "boost/format.hpp"
#include "./scope-manager.h"
#include "./literal-visitor.h"
#include "./cmdlike-visitor.h"

namespace sushi {
namespace code_generation {

#define EXPR_VISITOR_TRANSLATE_DEF(T, op) void Translate##op(      \
                        const T & lhs_visitor,                     \
                        const T & rhs_visitor,                     \
                        const ST & type)

constexpr char kMapVarCodeBeforeTemplate[] =
R"(local %1%=`declare -p %2%`
%1%=${%1%#*=}
%1%=${%1%:1:-1}
)";

struct ExprVisitor : public ast::ExpressionVisitor::Const {
    std::string val;
    std::string code_before;

    ExprVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment & environment,
        const scope::Scope * scope,
        bool is_left_value = false
        ) : is_left_value(is_left_value),
            scope_manager(scope_manager),
            environment(environment),
            scope(scope) {}

    using ST = TypeVisitor::SimplifiedType;
    SUSHI_VISITING(ast::Variable, variable) {
        auto new_name = scope_manager->FindNewName(variable.var.name, scope);
        if (is_left_value) {
            val = new_name;
        } else {
            auto type = environment.LookUp(&variable);
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);
            switch (type_visitor.type) {
            case ST::kInt:
            case ST::kBool:
            case ST::kUnit:
            case ST::kFd:
            case ST::kExitCode:
            case ST::kPath:
            case ST::kRelPath:
            case ST::kString:
            case ST::kChar:
            case ST::kFunc:
                val = '$' + new_name;
                break;
            case ST::kArray:
                val = (boost::format("\"${%1%[@]}\"") % new_name).str();
                break;
            case ST::kMap:
                auto temp = scope_manager->GetNewTemp(scope);
                code_before = (boost::format(kMapVarCodeBeforeTemplate) % temp % new_name).str();
                val = (boost::format("\"$%1%\"") % temp).str();
                break;
            }
        }
    }
    SUSHI_VISITING(ast::Literal, literal) {
        LiteralVisitor literal_visitor;
        literal.AcceptVisitor(literal_visitor);
        code_before = literal_visitor.code_before;
        val = literal_visitor.val;
    }
    SUSHI_VISITING(ast::UnaryExpr, unary_expr) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        unary_expr.expr->AcceptVisitor(expr_visitor);
        code_before = expr_visitor.code_before;
        using UOP = ast::UnaryExpr::Operator;
        switch (unary_expr.op) {
        case UOP::kNot:
            val = (boost::format("$((! %1%))") % expr_visitor.val).str();
            break;
        case UOP::kNeg:
            val = (boost::format("$((- %1%))") % expr_visitor.val).str();
            break;
        case UOP::kPos:
            val = (boost::format("`_sushi_abs_ %1%`") % expr_visitor.val).str();
            break;
        }
    }
    SUSHI_VISITING(ast::BinaryExpr, binary_expr) {
        ExprVisitor lhs_visitor(scope_manager, environment, scope);
        ExprVisitor rhs_visitor(scope_manager, environment, scope);
        binary_expr.lhs->AcceptVisitor(lhs_visitor);
        binary_expr.rhs->AcceptVisitor(rhs_visitor);

        // Get whole expression's type or (lhs or rhs)'s type?
        auto type = environment.LookUp(&binary_expr);
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        using BOP = ast::BinaryExpr::Operator;
        // Use macro to make code short...
        #define TRANSLATE_OP(op) Translate##op(lhs_visitor, rhs_visitor, type_visitor.type)

        switch (binary_expr.op) {
        case BOP::kAdd:     TRANSLATE_OP(Add); break;
        case BOP::kMinus:   TRANSLATE_OP(Minus); break;
        case BOP::kMult:    TRANSLATE_OP(Mult); break;
        case BOP::kDiv:     TRANSLATE_OP(Div); break;
        case BOP::kMod:     TRANSLATE_OP(Mod); break;
        case BOP::kLess:    TRANSLATE_OP(Less); break;
        case BOP::kGreat:   TRANSLATE_OP(Great); break;
        case BOP::kLessEq:  TRANSLATE_OP(LessEq); break;
        case BOP::kGreatEq: TRANSLATE_OP(GreatEq); break;
        case BOP::kEqual:   TRANSLATE_OP(Equal); break;
        case BOP::kNotEq:   TRANSLATE_OP(NotEq); break;
        case BOP::kAnd:     TRANSLATE_OP(And); break;
        case BOP::kOr:      TRANSLATE_OP(Or); break;
        }

        #undef TRANSLATE_OP
    }
    SUSHI_VISITING(ast::CommandLike, cmd_like) {
        CmdLikeVisitor cmdlike_visitor(scope_manager, environment, scope);
        cmd_like.AcceptVisitor(cmdlike_visitor);
        code_before = cmdlike_visitor.code_before;
        val = cmdlike_visitor.val;
    }
    SUSHI_VISITING(ast::Indexing, indexing) {
        ExprVisitor indexable_visitor(
            scope_manager,
            environment,
            scope,
            true);
        ExprVisitor index_visitor(
            scope_manager,
            environment,
            scope,
            false);
        indexing.indexable->AcceptVisitor(indexable_visitor);
        indexing.index->AcceptVisitor(index_visitor);

        code_before += indexable_visitor.code_before + '\n' + index_visitor.code_before;
        val = indexable_visitor.val + '[' + index_visitor.val + ']';
    }

  protected:
    bool is_left_value;
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment & environment;
    const scope::Scope * scope;

    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Add);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Minus);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Mult);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Div);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Mod);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Less);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Great);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, LessEq);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, GreatEq);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Equal);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, NotEq);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, And);
    EXPR_VISITOR_TRANSLATE_DEF(ExprVisitor, Or);
};

struct ConditionExprVisitor : public ExprVisitor {
    /*
     * Translate
     * Bool to `condition`, it is wrapped in `[[ ]]`
     */
    ConditionExprVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment & environment,
        const scope::Scope * scope
        ) : ExprVisitor(
            scope_manager,
            environment,
            scope,
            false
        ) {}

    SUSHI_VISITING(ast::Variable, variable) {
        auto new_name = scope_manager->FindNewName(variable.var.name, scope);
        constexpr char template_[] = "($%1% -ne 0)";
        val = (boost::format(template_) % new_name).str();
    }
    SUSHI_VISITING(ast::Literal, literal) {
        ConditionLiteralVisitor literal_visitor;
        literal.AcceptVisitor(literal_visitor);
        code_before = literal_visitor.code_before;
        val = literal_visitor.val;
    }
    SUSHI_VISITING(ast::UnaryExpr, unary_expr) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        unary_expr.expr->AcceptVisitor(expr_visitor);
        code_before = expr_visitor.code_before;
        // Only ! will be here
        using UOP = ast::UnaryExpr::Operator;
        switch (unary_expr.op) {
        case UOP::kNot:
            val = (boost::format("(! %1% -ne 0)") % expr_visitor.val).str();
        }
    }
    SUSHI_VISITING(ast::BinaryExpr, binary_expr) {
        ExprVisitor lhs_visitor(scope_manager, environment, scope);
        ExprVisitor rhs_visitor(scope_manager, environment, scope);
        binary_expr.lhs->AcceptVisitor(lhs_visitor);
        binary_expr.rhs->AcceptVisitor(rhs_visitor);

        // Get whole expression's type or (lhs or rhs)'s type?
        auto type = environment.LookUp(&binary_expr);
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        using BOP = ast::BinaryExpr::Operator;
        // Use macro to make code short...
        #define TRANSLATE_OP(op) Translate##op(lhs_visitor, rhs_visitor, type_visitor.type)

        // < > <= >= == != and or
        switch (binary_expr.op) {
        case BOP::kLess:    TRANSLATE_OP(Less); break;
        case BOP::kGreat:   TRANSLATE_OP(Great); break;
        case BOP::kLessEq:  TRANSLATE_OP(LessEq); break;
        case BOP::kGreatEq: TRANSLATE_OP(GreatEq); break;
        case BOP::kEqual:   TRANSLATE_OP(Equal); break;
        case BOP::kNotEq:   TRANSLATE_OP(NotEq); break;
        case BOP::kAnd:     TRANSLATE_OP(And); break;
        case BOP::kOr:      TRANSLATE_OP(Or); break;
        }

        #undef TRANSLATE_OP
    }
    // CommanLike can be condition directly
    // SUSHI_VISITING(ast::CommandLike, cmd_like);

    SUSHI_VISITING(ast::Indexing, indexing) {
        // indexable is Array Bool
        ExprVisitor indexable_visitor(
            scope_manager,
            environment,
            scope,
            true);
        ExprVisitor index_visitor(
            scope_manager,
            environment,
            scope,
            false);
        indexing.indexable->AcceptVisitor(indexable_visitor);
        indexing.index->AcceptVisitor(index_visitor);

        code_before += indexable_visitor.code_before + '\n' + index_visitor.code_before;
        val = (boost::format("%1%[%2%] -ne 0") % indexable_visitor.val % index_visitor.val).str();
    }
};

struct SwitchCaseExprVisitor : public ExprVisitor {
    /*
     * Translate
     * Bool to `[[ condition ]]`
     * Function Bool to `func_name <t_Bool>`
     */
    SwitchCaseExprVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment & environment,
        const scope::Scope * scope,
        const std::string & case_val
        ) : ExprVisitor(
            scope_manager,
            environment,
            scope,
            false
        ), case_val(case_val) {}

  protected:
    const std::string & case_val;
};

} // namespace code_generation
} // namespace sushi


#endif
