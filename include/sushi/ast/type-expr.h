#ifndef SUSHI_AST_TYPE_H_
#define SUSHI_AST_TYPE_H_

#include "sushi/type-system/type.h"
#include "sushi/util/visitor.h"
#include <memory>
#include <utility>

namespace sushi {
namespace ast {

struct TypeLit;
struct ArrayType;
struct MapType;
// struct FunctionType;

using TypeExprVisitor = sushi::util::DefineVisitor<TypeLit, ArrayType, MapType>;

// TODO: refactor after adding higher kinding type

struct TypeExpr {
    SUSHI_VISITABLE(TypeExprVisitor);

    virtual ~TypeExpr() = default;
};

struct TypeLit : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    TypeLit(type::BuiltInAtom::Type type) : type(type) {}
    type::BuiltInAtom::Type type;
};

struct ArrayType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    ArrayType(type::BuiltInAtom::Type element) : element(std::move(element)) {}

    type::BuiltInAtom::Type element;
};

struct MapType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    MapType(type::BuiltInAtom::Type key, type::BuiltInAtom::Type value)
        : key(key), value(value) {}

    type::BuiltInAtom::Type key;
    type::BuiltInAtom::Type value;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_TYPE_H_
