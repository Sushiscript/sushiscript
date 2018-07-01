#ifndef SUSHI_SCOPE_TEST_EXPRESSION_VISITOR_H_
#define SUSHI_SCOPE_TEST_EXPRESSION_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope/scope.h"
#include "sushi/scope/environment.h"
#include "sushi/scope/error.h"

namespace sushi {
namespace scope {
namespace test {

struct ExpressionVisitor : public ast::ExpressionVisitor::Const {
    ExpressionVisitor() {}

    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unary_expr);
    SUSHI_VISITING(ast::BinaryExpr, binary_expr);
    SUSHI_VISITING(ast::CommandLike, cmd_like);
    SUSHI_VISITING(ast::Indexing, indexing);

    std::vector<const ast::Program*> programs;
    std::vector<const ast::Identifier*> identifiers;
};

}
} // namespace scope
} // namespace sushi

#endif
