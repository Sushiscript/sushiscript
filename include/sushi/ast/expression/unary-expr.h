#ifndef SUSHI_AST_EXPRESSION_UNARY_EXPR_H
#define SUSHI_AST_EXPRESSION_UNARY_EXPR_H

#include "./expression.h"

namespace sushi {

class UnaryExpr : public Expression {
  public:
    SUSHI_ACCEPT_VISITOR(Expression)

  private:
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_UNARY_EXPR_H
