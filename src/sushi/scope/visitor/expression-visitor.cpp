#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void ExpressionVisitor::Visit(const T &t)

VISIT(ast::Variable, variable) {
    auto info = Scope::CreateIdentInfo(
        variable.start_location, scope.get());
    scope->Insert(variable.var.name, info);
}

VISIT(ast::Literal, literal) {
    LiteralVisitor literal_visitor(environment, scope);
    literal.AcceptVisitor(literal_visitor);
}

VISIT(ast::UnaryExpr, unary_expr) {
    ExpressionVisitor expression_visitor(environment, scope);
    unary_expr.expr->AcceptVisitor(expression_visitor);
}

VISIT(ast::BinaryExpr, binary_expr) {
    ExpressionVisitor expression_visitor(environment, scope);
    binary_expr.lhs->AcceptVisitor(expression_visitor);
    binary_expr.rhs->AcceptVisitor(expression_visitor);
}

VISIT(ast::CommandLike, cmd_like) {
    CommandLikeVisitor command_like_visitor(environment, scope);
    cmd_like.AcceptVisitor(command_like_visitor);
}

VISIT(ast::Indexing, indexing) {
    ExpressionVisitor expression_visitor(environment, scope);
    indexing.index->AcceptVisitor(expression_visitor);
    indexing.indexable->AcceptVisitor(expression_visitor);
}

} // namespace scope
} // namespace sushi
