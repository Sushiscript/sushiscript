#ifndef SUSHI_AST_EXPRESSION_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_EXPRESSION_H_

#include "sushi/util/visitor.h"

namespace sushi {
namespace ast {

struct Variable;
struct Literal;
struct UnaryExpr;
struct BinaryExpr;
struct CommandLike;

using ExpressionVisitor = sushi::util::DefineVisitor<
    Variable, Literal, UnaryExpr, BinaryExpr, CommandLike>;

struct Expression {
    SUSHI_VISITABLE(ExpressionVisitor)
};

} // namespace ast
} // namespace sushi

#endif // namespace SUSHI_AST_EXPRESSION_EXPRESSION_H_
