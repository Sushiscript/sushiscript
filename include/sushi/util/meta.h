#ifndef SUSHI_UTIL_META_H_
#define SUSHI_UTIL_META_H_

namespace sushi {
namespace util {

template <bool b, typename T, typename F>
struct if_ {
    using type = T;
};
template <typename T, typename F>
struct if_<false, T, F> {
    using type = F;
};
template <bool b, typename T, typename F>
using if_t = typename if_<b, T, F>::type;

} // namespace util
} // namespace sushi

#endif // SUSHI_UTIL_META_H_
