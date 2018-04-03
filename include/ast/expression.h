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

class ExpressionVisitor;

class Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) = 0;

  private:
};

class Variable;
class Literal;
class UnaryExpr;
class BinaryExpr;
class CommandLike;

class ExpressionVisitor {
  public:
    virtual void Visit(Variable *) = 0;
    virtual void Visit(Literal *) = 0;
    virtual void Visit(UnaryExpr *) = 0;
    virtual void Visit(BinaryExpr *) = 0;
    virtual void Visit(CommandLike *) = 0;
};

class Variable : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::string name_;
};

class Literal : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
};

class UnaryExpr : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
};

class BinaryExpr : public Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::unique_ptr<Expression> lhs_, rhs_;
};

class Redirection {
  public:
  private:
};

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

#endif // SUSHI_AST_EXPRESSION_H_
