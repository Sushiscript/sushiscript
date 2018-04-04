#ifndef SUSHI_AST_EXPRESSION_BINARY_EXPR_H
#define SUSHI_AST_EXPRESSION_BINARY_EXPR_H

#include "./expression.h"
#include <memory>

namespace sushi {

class BinaryExpr : public Expression {
  public:
    SUSHI_ACCEPT_VISITOR(Expression)

  private:
    std::unique_ptr<Expression> lhs_, rhs_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_BINARY_EXPR_H
