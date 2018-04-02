#ifndef SUSHI_AST_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_H_

#include <memory>
#include <string>
#include <vector>

namespace sushi {

class Type {
  public:
  private:
};

class Expression {
  public:
    virtual Type Type() = 0;
};

class Variable : public Expression {
  public:
  private:
    std::string name_;
};

class Literal : public Expression {
  public:
  private:
};

class UnaryExpr : public Expression {
  public:
  private:
};

class BinaryExpr : public Expression {
  public:
  private:
    std::unique_ptr<Expression> lhs_, rhs_;
};

class Redirection {
  public:
  private:
};

class CommandLike : public Expression {
  public:
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

#endif // SUSHI_AST_EXPRESSION_H_
