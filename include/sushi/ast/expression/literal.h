#ifndef SUSHI_AST_EXPRESSION_LITERAL_H_
#define SUSHI_AST_EXPRESSION_LITERAL_H_

#include "./expression.h"

namespace sushi {

class Literal : public Expression {
  public:
    SUSHI_ACCEPT_VISITOR(Expression)

  private:
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_LITERAL_H_
