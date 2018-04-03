#ifndef SUSHI_AST_EXPRESSION_LITERAL_H_
#define SUSHI_AST_EXPRESSION_LITERAL_H_

#include "./expression.h"
#include "./visitor.h"

namespace sushi {

class Literal : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_LITERAL_H_
