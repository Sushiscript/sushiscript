#ifndef SUSHI_SCOPE_EXPRESSION_VISITOR_H_
#define SUSHI_SCOPE_EXPRESSION_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope/environment.h"
#include "literal-visitor.h"

namespace sushi {
namespace scope {

struct ExpressionVisitor : public ast::ExpressionVisitor::Const {
    Environment &environment;
    std::shared_ptr<Scope> &scope;

    ExpressionVisitor(Environment &environment, std::shared_ptr<Scope> &scope)
        : environment(environment), scope(scope){};

    SUSHI_VISITING(ast::Variable, variable) {
        Scope::IdentInfo info = { variable.start_location.get() , scope.get() };
        scope->Insert(variable.var.name, info);
    }

    SUSHI_VISITING(ast::Literal, literal) {
        LiteralVisitor literal_visitor (environment, scope);
        literal.AcceptVisitor(literal_visitor);
    }

    SUSHI_VISITING(ast::UnaryExpr, unary_expr) {
        ExpressionVisitor expression_visitor(environment, scope);
        unary_expr.expr->AcceptVisitor(expression_visitor);
    }

    SUSHI_VISITING(ast::BinaryExpr, binary_expr) {
        ExpressionVisitor expression_visitor(environment, scope);
        binary_expr.lhs->AcceptVisitor(expression_visitor);
        binary_expr.rhs->AcceptVisitor(expression_visitor);
    }

    SUSHI_VISITING(ast::CommandLike, cmd_like) {

    }

    SUSHI_VISITING(ast::Indexing, indexing) {
        ExpressionVisitor expression_visitor(environment, scope);
        indexing.index->AcceptVisitor(expression_visitor);
        indexing.indexable->AcceptVisitor(expression_visitor);

    }
};

} // namespace scope
} // namespace sushi

#endif
