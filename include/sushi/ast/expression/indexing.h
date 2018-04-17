#ifndef SUSHI_AST_EXPRESSION_INDEXING_H_
#define SUSHI_AST_EXPRESSION_INDEXING_H_

#include "./expression.h"
#include "sushi/util/visitor.h"
#include <memory>

namespace sushi {
namespace ast {

struct Indexing : Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    Indexing(
        std::unique_ptr<Expression> indexable,
        std::unique_ptr<Expression> index)
        : indexable(std::move(indexable)), index(std::move(index)) {}

    std::unique_ptr<Expression> indexable;
    std::unique_ptr<Expression> index;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_INDEXING_H_
