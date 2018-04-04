#ifndef SUSHI_AST_EXPRESSION_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_EXPRESSION_H_

#include "sushi/util/visitor.h"

namespace sushi {

class Variable;
class Literal;
class UnaryExpr;
class BinaryExpr;
class CommandLike;

using ExpressionVisitor =
    sushi::util::Visitor<Variable, Literal, UnaryExpr, BinaryExpr, CommandLike>;

class Expression {
  public:
    SUSHI_VISITABLE(ExpressionVisitor)
};

} // namespace sushi

#endif // namespace SUSHI_AST_EXPRESSION_EXPRESSION_H_
