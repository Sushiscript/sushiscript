#ifndef SUSHI_UTIL_VISITOR_H_
#define SUSHI_UTIL_VISITOR_H_

namespace sushi {
namespace util {

namespace detail {

template <typename... Ts>
struct VisitorImpl;

template <typename T>
struct VisitorImpl<T> {
    virtual void Visit(T &) = 0;
};

template <typename T, typename... Ts>
struct VisitorImpl<T, Ts...> : VisitorImpl<Ts...> {
    using VisitorImpl<Ts...>::Visit;
    virtual void Visit(T &) = 0;
};

} // namespace detail

template <typename... Ts>
struct Visitor : detail::VisitorImpl<Ts...> {
    virtual ~Visitor<Ts...>() {}
};

#define SUSHI_ACCEPT_VISITOR(B)                                                \
    virtual void AcceptVisitor(B::TheVisitor &v) override {                    \
        v.Visit(*this);                                                        \
    }

#define SUSHI_VISITABLE(v)                                                     \
    using TheVisitor = v;                                                      \
    virtual void AcceptVisitor(TheVisitor &) = 0;

#define SUSHI_VISITING(t, p) virtual void Visit(t &p) override

} // namespace util

} // namespace sushi

#endif // SUSHI_UTIL_VISITOR_H_
