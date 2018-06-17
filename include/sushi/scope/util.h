#ifndef SUSHI_SCOPE_UTIL_H_
#define SUSHI_SCOPE_UTIL_H_

#include <memory>
#include <unordered_map>

namespace sushi {
namespace scope {

// Table is a short name for std::unordered_map
template <typename KeyType, typename ValueType>
using Table = std::unordered_map<KeyType, ValueType>;

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_UTIL_H_
