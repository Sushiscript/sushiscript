#ifndef SUSHI_AST_EXPRESSION_VARIABLE_H_
#define SUSHI_AST_EXPRESSION_VARIABLE_H_

#include "./expression.h"
#include <string>

namespace sushi {

class Variable : public Expression {
  public:
    SUSHI_ACCEPT_VISITOR(Expression)

  private:
    std::string name_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_VARIABLE_H_
