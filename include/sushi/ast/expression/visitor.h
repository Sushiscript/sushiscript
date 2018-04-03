#ifndef SUSHI_AST_EXPRESSION_VISITOR_H_
#define SUSHI_AST_EXPRESSION_VISITOR_H_

namespace sushi {

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

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_VISITOR_H_
