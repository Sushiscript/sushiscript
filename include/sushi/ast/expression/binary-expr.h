#ifndef SUSHI_AST_EXPRESSION_BINARY_EXPR_H
#define SUSHI_AST_EXPRESSION_BINARY_EXPR_H

#include "./expression.h"
#include "./visitor.h"
#include <memory>

namespace sushi {

class BinaryExpr : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::unique_ptr<Expression> lhs_, rhs_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_BINARY_EXPR_H
