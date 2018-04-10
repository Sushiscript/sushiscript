#ifndef SUSHI_AST_EXPRESSION_BINARY_EXPR_H_
#define SUSHI_AST_EXPRESSION_BINARY_EXPR_H_

#include "./expression.h"
#include <memory>

namespace sushi {
namespace ast {

struct BinaryExpr : Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    enum class Operator {
        kAdd,
        kMinus,
        kMult,
        kDiv,
        kMod,
        kLess,
        kGreat,
        kLessEq,
        kGreatEq,
        kEqual,
        kNotEq,
        kAnd,
        kOr
    };

    BinaryExpr(
        std::unique_ptr<Expression> lhs, BinaryExpr::Operator op,
        std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

    std::unique_ptr<Expression> lhs, rhs;
    BinaryExpr::Operator op;
};

} // namespace ast

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_BINARY_EXPR_H_
