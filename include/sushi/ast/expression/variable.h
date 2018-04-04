#ifndef SUSHI_AST_EXPRESSION_VARIABLE_H_
#define SUSHI_AST_EXPRESSION_VARIABLE_H_

#include "./expression.h"
#include <string>

namespace sushi {

struct Variable : public Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    Variable(const std::string &name) : name(name) {}

    std::string name;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_VARIABLE_H_
