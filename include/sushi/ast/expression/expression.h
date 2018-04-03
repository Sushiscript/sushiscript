#ifndef SUSHI_AST_EXPRESSION_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_EXPRESSION_H_

namespace sushi {

class ExpressionVisitor;

class Expression {
  public:
    virtual void AcceptVisitor(ExpressionVisitor &visitor) = 0;

  private:
};

} // namespace sushi

#endif // namespace SUSHI_AST_EXPRESSION_EXPRESSION_H_
