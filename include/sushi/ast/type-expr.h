#ifndef SUSHI_AST_TYPE_H_
#define SUSHI_AST_TYPE_H_

#include "sushi/util/visitor.h"
#include <memory>
#include <utility>

namespace sushi {

struct TypeLit;
struct ArrayType;
struct MapType;
// struct FunctionType;

using TypeExprVisitor = sushi::util::DefineVisitor<TypeLit, ArrayType, MapType>;

// TODO: refactor after adding higher kinding type

struct TypeExpr {
    SUSHI_VISITABLE(TypeExprVisitor);
};

struct TypeLit : public TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    enum class Value { kInt, kBool, kUnit, kFd, kExitCode, kPath, kString };
    TypeLit(TypeLit::Value type) : type(type) {}
    TypeLit::Value type;
};

struct ArrayType : public TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    ArrayType(std::unique_ptr<TypeExpr> element)
        : element(std::move(element)) {}

    std::unique_ptr<TypeExpr> element;
};

struct MapType : public TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    MapType(TypeLit::Value key, std::unique_ptr<TypeExpr> value)
        : key(key), value(std::move(value)) {}

    TypeLit::Value key;
    std::unique_ptr<TypeExpr> value;
};

} // namespace sushi

#endif // SUSHI_AST_TYPE_H_
