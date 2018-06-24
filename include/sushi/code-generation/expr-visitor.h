#ifndef SUSHI_CODE_GEN_EXPR_VISITOR_H_
#define SUSHI_CODE_GEN_EXPR_VISITOR_H_

#include "./cmdlike-visitor.h"
#include "./literal-visitor.h"
#include "./scope-manager.h"
#include "./type-visitor.h"
#include "boost/format.hpp"
#include "sushi/ast.h"
#include "sushi/scope.h"

#include <unordered_set>

namespace sushi {
namespace code_generation {

#define EXPR_VISITOR_TRANSLATE_DEF(T, op)                                      \
    void Translate##op(                                                        \
        const T &lhs_visitor, const T &rhs_visitor, const ST &type)

#define EXPR_VISITOR_TRANSLATE_IMPL(T, op)                                     \
    void ExprVisitor::Translate##op(                                           \
        const T &lhs_visitor, const T &rhs_visitor, const ST &type)

constexpr char kMapVarCodeBeforeTemplate[] =
    R"(local %1%=`declare -p %2%`
%1%=${%1%#*=}
%1%=${%1%:1:-1}
)";

struct ExprVisitor : public ast::ExpressionVisitor::Const {
    std::string val;
    std::string code_before;
    std::string raw_id;

    std::unordered_set<std::string> new_ids;

    ExprVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment &environment, const scope::Scope *scope,
        bool is_left_value = false)
        : is_left_value(is_left_value), scope_manager(scope_manager),
          environment(environment), scope(scope) {}

    using ST = TypeVisitor::SimplifiedType;
    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unary_expr);
    SUSHI_VISITING(ast::BinaryExpr, binary_expr);
    SUSHI_VISITING(ast::CommandLike, cmd_like);
    SUSHI_VISITING(ast::Indexing, indexing);

  protected:
    bool is_left_value;
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment &environment;
    const scope::Scope *scope;

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

} // namespace code_generation
} // namespace sushi

#endif
