#ifndef SUSHI_SCOPE_UTIL_H_
#define SUSHI_SCOPE_UTIL_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace sushi {
namespace scope {

namespace debug {

template <class T>
void log(T variable, std::string message) {
    std::cout << message << " " << variable << std::endl;
}

} // namespace debug

// Table is a short name for std::unordered_map
template <typename KeyType, typename ValueType>
using Table = std::unordered_map<KeyType, ValueType>;


} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_UTIL_H_
