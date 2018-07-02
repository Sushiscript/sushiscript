#ifndef SUSHI_CODE_GEN_UTIL_H_
#define SUSHI_CODE_GEN_UTIL_H_

#include "sushi/scope/visitor/type-visitor.h"
#include "sushi/scope.h"
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

}
}

#endif
