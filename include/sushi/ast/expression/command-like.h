#ifndef SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
#define SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_

#include "./expression.h"
#include "./visitor.h"
#include <string>
#include <vector>

namespace sushi {

class Redirection {};

class CommandLike : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::vector<Redirection> redirects_;
};

class FunctionCall : public CommandLike {
  public:
  private:
    std::string func_name_;
    std::vector<std::unique_ptr<Expression>> paramters_;
};

class Command : public CommandLike {
  public:
  private:
    std::string cmd_name_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
