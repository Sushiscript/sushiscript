#ifndef SUSHI_AST_EXPRESSION_VARIABLE_H_
#define SUSHI_AST_EXPRESSION_VARIABLE_H_

#include "./expression.h"
#include "./identifier.h"
#include <string>

namespace sushi {
namespace ast {

struct Variable : Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    Variable(Identifier var) : var(std::move(var)) {}

    Identifier var;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_VARIABLE_H_
