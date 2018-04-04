#ifndef SUSHI_AST_EXPRESSION_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_EXPRESSION_H_

#include "sushi/util/visitor.h"

namespace sushi {

struct Variable;
struct Literal;
struct UnaryExpr;
struct BinaryExpr;
struct CommandLike;

using ExpressionVisitor = sushi::util::DefineVisitor<
    Variable, Literal, UnaryExpr, BinaryExpr, CommandLike>;

struct Expression {
  public:
    SUSHI_VISITABLE(ExpressionVisitor)
};

} // namespace sushi

#endif // namespace SUSHI_AST_EXPRESSION_EXPRESSION_H_
