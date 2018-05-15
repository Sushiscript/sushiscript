#ifndef SUSHI_AST_EXPRESSION_EXPRESSION_H_
#define SUSHI_AST_EXPRESSION_EXPRESSION_H_

#include "sushi/ast/statement/statement.h"
#include "sushi/util/visitor.h"

namespace sushi {
namespace ast {

struct Variable;
struct Literal;
struct UnaryExpr;
struct BinaryExpr;
struct CommandLike;
struct Indexing;

using ExpressionVisitor = sushi::util::DefineVisitor<
    Variable, Literal, UnaryExpr, BinaryExpr, CommandLike, Indexing>;

struct Expression : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)
    SUSHI_VISITABLE(ExpressionVisitor)
};

} // namespace ast
} // namespace sushi

#endif // namespace SUSHI_AST_EXPRESSION_EXPRESSION_H_
