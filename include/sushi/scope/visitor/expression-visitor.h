#ifndef SUSHI_SCOPE_EXPRESSION_VISITOR_H_
#define SUSHI_SCOPE_EXPRESSION_VISITOR_H_

#include "sushi/ast.h"

namespace sushi {
namespace scope {

struct ExpressionVisitor : public ast::ExpressionVisitor::Const {

    SUSHI_VISITING(ast::Variable, variable) {
    }
    SUSHI_VISITING(ast::Literal, literal) {
    }
    SUSHI_VISITING(ast::UnaryExpr, unary_expr) {
    }
    SUSHI_VISITING(ast::BinaryExpr, binary_expr) {
    }
    SUSHI_VISITING(ast::CommandLike, cmd_like) {
    }
    SUSHI_VISITING(ast::Indexing, indexing) {
    }
};

} // namespace scope
} // namespace sushi


#endif
