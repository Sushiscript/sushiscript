#include "sushi/scope.h"
#include "../include/expression-visitor.h"
#include "../include/literal-visitor.h"
#include "../include/command-like-visitor.h"

namespace sushi {
namespace scope {
namespace test {

#define VISIT(T, t) void ExpressionVisitor::Visit(const T &t)

VISIT(ast::Variable, variable) {
    identifiers.push_back(&variable.var);
}

VISIT(ast::Literal, literal) {
    test::LiteralVisitor lit_visitor;
    literal.AcceptVisitor(lit_visitor);
    MergeVector(identifiers, lit_visitor.identifiers);
}

VISIT(ast::UnaryExpr, unary_expr) {
    ExpressionVisitor expression_visitor;
    unary_expr.expr->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::BinaryExpr, binary_expr) {
    ExpressionVisitor expression_visitor;
    binary_expr.lhs->AcceptVisitor(expression_visitor);
    binary_expr.rhs->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::CommandLike, cmd_like) {
    CommandLikeVisitor command_like_visitor;
    cmd_like.AcceptVisitor(command_like_visitor);
    MergeVector(identifiers, command_like_visitor.identifiers);
}

VISIT(ast::Indexing, indexing) {
    ExpressionVisitor expression_visitor;
    indexing.index->AcceptVisitor(expression_visitor);
    indexing.indexable->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

}
} // namespace scope
} // namespace sushi
