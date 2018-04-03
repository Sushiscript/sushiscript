#ifndef SUSHI_AST_EXPRESSION_VARIABLE_H_
#define SUSHI_AST_EXPRESSION_VARIABLE_H_

#include "./expression.h"
#include "./visitor.h"
#include <string>

namespace sushi {

class Variable : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::string name_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_VARIABLE_H_
