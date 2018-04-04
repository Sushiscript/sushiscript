#ifndef SUSHI_UTIL_VISITOR_H_
#define SUSHI_UTIL_VISITOR_H_

namespace sushi {
namespace util {

namespace detail {

template <bool kConst, typename T>
struct Ref {
    using Type = const T &;
};

template <typename T>
struct Ref<false, T> {
    using Type = T &;
};

template <bool kConst, typename... Ts>
struct VisitorImpl;

template <bool kConst, typename T>
struct VisitorImpl<kConst, T> {
    virtual void Visit(typename Ref<kConst, T>::Type) = 0;
};

template <bool kConst, typename T, typename... Ts>
struct VisitorImpl<kConst, T, Ts...> : VisitorImpl<kConst, Ts...> {
    using VisitorImpl<kConst, Ts...>::Visit;
    virtual void Visit(typename Ref<kConst, T>::Type) = 0;
};

} // namespace detail

template <bool kConst, typename... Ts>
struct Visitor : detail::VisitorImpl<kConst, Ts...> {
    using Const = Visitor<true, Ts...>;
    using Mutable = Visitor<false, Ts...>;
    virtual ~Visitor<kConst, Ts...>() {}

  protected:
    template <typename T>
    struct VisitorToParam {
        using Type = typename detail::Ref<kConst, T>::Type;
    };
};

template <typename... Ts>
using ConstVisitor = Visitor<true, Ts...>;

template <typename... Ts>
using MutableVisitor = Visitor<false, Ts...>;

template <typename... Ts>
struct DefineVisitor {
    using Const = ConstVisitor<Ts...>;
    using Mutable = MutableVisitor<Ts...>;
};

#define SUSHI_VISITABLE(V)                                                     \
  protected:                                                                   \
    using ConstVisitor = V::Const;                                             \
    using MutableVisitor = V::Mutable;                                         \
                                                                               \
  public:                                                                      \
    virtual void AcceptVisitor(ConstVisitor &) const = 0;                      \
    virtual void AcceptVisitor(ConstVisitor &&) const = 0;                     \
    virtual void AcceptVisitor(MutableVisitor &) = 0;                          \
    virtual void AcceptVisitor(MutableVisitor &&) = 0;

#define SUSHI_ACCEPT_VISITOR_FROM(B)                                           \
    virtual void AcceptVisitor(B::ConstVisitor &v) const override {            \
        v.Visit(*this);                                                        \
    }                                                                          \
    virtual void AcceptVisitor(B::ConstVisitor &&v) const override {           \
        v.Visit(*this);                                                        \
    }                                                                          \
    virtual void AcceptVisitor(B::MutableVisitor &v) override {                \
        v.Visit(*this);                                                        \
    }                                                                          \
    virtual void AcceptVisitor(B::MutableVisitor &&v) override {               \
        v.Visit(*this);                                                        \
    }

#define SUSHI_VISITING(T, t)                                                   \
    virtual void Visit(VisitorToParam<T>::Type t) override

} // namespace util

} // namespace sushi

#endif // SUSHI_UTIL_VISITOR_H_
