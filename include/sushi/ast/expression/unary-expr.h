#ifndef SUSHI_AST_EXPRESSION_UNARY_EXPR_H
#define SUSHI_AST_EXPRESSION_UNARY_EXPR_H

#include "./expression.h"
#include "./visitor.h"

namespace sushi {

class UnaryExpr : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_UNARY_EXPR_H
