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
};

struct TypeLit : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    TypeLit(type::BuiltInAtom::Type type) : type(type) {}
    type::BuiltInAtom::Type type;
};

struct ArrayType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    ArrayType(std::unique_ptr<TypeExpr> element)
        : element(std::move(element)) {}

    std::unique_ptr<TypeExpr> element;
};

struct MapType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    MapType(type::BuiltInAtom::Type key, std::unique_ptr<TypeExpr> value)
        : key(key), value(std::move(value)) {}

    type::BuiltInAtom::Type key;
    std::unique_ptr<TypeExpr> value;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_TYPE_H_
