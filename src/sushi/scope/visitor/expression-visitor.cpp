#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void ExpressionVisitor::Visit(const T &t)

VISIT(ast::Variable, variable) {
    auto var_id = scope->LookUp(variable.var.name);
    if (var_id == nullptr) {
        errs.push_back(Error(Error::Type::kUndefinedError, variable.var.name));
        return;
    }
    environment.Insert(&variable.var, scope);
}

VISIT(ast::Literal, literal) {
    LiteralVisitor literal_visitor(environment, scope);
    literal.AcceptVisitor(literal_visitor);
    MergeVector(errs, literal_visitor.errs);
}

VISIT(ast::UnaryExpr, unary_expr) {
    ExpressionVisitor expression_visitor(environment, scope);
    unary_expr.expr->AcceptVisitor(expression_visitor);
    MergeVector(errs, expression_visitor.errs);
}

VISIT(ast::BinaryExpr, binary_expr) {
    ExpressionVisitor expression_visitor(environment, scope);
    binary_expr.lhs->AcceptVisitor(expression_visitor);
    binary_expr.rhs->AcceptVisitor(expression_visitor);
    MergeVector(errs, expression_visitor.errs);
}

VISIT(ast::CommandLike, cmd_like) {
    CommandLikeVisitor command_like_visitor(environment, scope);
    cmd_like.AcceptVisitor(command_like_visitor);
    MergeVector(errs, command_like_visitor.errs);
}

VISIT(ast::Indexing, indexing) {
    ExpressionVisitor expression_visitor(environment, scope);
    indexing.index->AcceptVisitor(expression_visitor);
    indexing.indexable->AcceptVisitor(expression_visitor);
    MergeVector(errs, expression_visitor.errs);
}

} // namespace scope
} // namespace sushi
