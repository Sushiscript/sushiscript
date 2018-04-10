#ifndef SUSHI_AST_EXPRESSION_UNARY_EXPR_H
#define SUSHI_AST_EXPRESSION_UNARY_EXPR_H

#include "./expression.h"
#include <memory>

namespace sushi {
namespace ast {

struct UnaryExpr : Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    enum struct Operator { kNot, kNeg, kPos };

    UnaryExpr(std::unique_ptr<Expression> expr, UnaryExpr::Operator op)
        : expr(std::move(expr)), op(op) {}

    std::unique_ptr<Expression> expr;
    Operator op;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_UNARY_EXPR_H
