#ifndef SUSHI_UTIL_OPTIONAL_H_
#define SUSHI_UTIL_OPTIONAL_H_

#include "boost/optional.hpp"
#include <type_traits>

namespace sushi {
namespace util {

namespace detail {

template <typename T>
struct IsOptional : std::false_type {};

template <typename T>
struct IsOptional<boost::optional<T>> : std::true_type {};

} // namespace detail

namespace monadic_optional {

template <
    typename T, typename F,
    typename = std::enable_if_t<std::is_convertible<
        decltype(std::declval<F>()()), boost::optional<T>>::value>>
boost::optional<T> operator|(boost::optional<T> opt, F f) {
    if (not opt) {
        return f();
    }
    return std::move(opt);
}

template <typename T, typename F>
boost::optional<std::result_of_t<F(T)>> operator>(boost::optional<T> opt, F f) {
    if (not opt) {
        return boost::none;
    }
    return f(std::move(*opt));
}

} // namespace monadic_optional

} // namespace util
} // namespace sushi

#endif // SUSHI_UTIL_OPTIONAL_H_
