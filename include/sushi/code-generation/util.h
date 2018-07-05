#ifndef SUSHI_CODE_GEN_UTIL_H_
#define SUSHI_CODE_GEN_UTIL_H_

#include "sushi/scope.h"
#include "sushi/code-generation/visitor/type-visitor.h"
#include <unordered_set>

template <typename T>
inline void
MergeSets(std::unordered_set<T> &lhs, const std::unordered_set<T> &rhs) {
    lhs.insert(rhs.begin(), rhs.end());
}

namespace sushi {
namespace code_generation {

using ST = TypeVisitor::SimplifiedType;

inline ST GetType(const scope::Environment &env, const ast::Expression *expr) {
    auto type = env.LookUp(expr);
    TypeVisitor visitor;
    type->AcceptVisitor(visitor);
    return visitor.type;
}

inline ST GetTypeFromTypeExpr(const ast::TypeExpr *type_expr) {
    TypeExprVisitor visitor;
    type_expr->AcceptVisitor(visitor);
    return visitor.type;
}

} // namespace code_generation
} // namespace sushi

#endif
