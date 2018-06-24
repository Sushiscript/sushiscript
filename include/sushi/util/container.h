#ifndef SUSHI_UTIL_CONTAINER_H_
#define SUSHI_UTIL_CONTAINER_H_

#include <algorithm>
#include <vector>

namespace sushi {

namespace util {

template <typename T>
bool Has(const std::vector<T> &v, const T &t) {
    return std::find(begin(v), end(v), t) != end(v);
}

} // namespace util

} // namespace sushi

#endif // SUSHI_UTIL_CONTAINER_H_