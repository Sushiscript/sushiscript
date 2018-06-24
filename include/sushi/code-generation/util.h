#ifndef SUSHI_CODE_GEN_UTIL_H_
#define SUSHI_CODE_GEN_UTIL_H_

#include <unordered_set>

template <typename T>
inline void
MergeSets(std::unordered_set<T> &lhs, const std::unordered_set<T> &rhs) {
    lhs.insert(rhs.begin(), rhs.end());
}

#endif
