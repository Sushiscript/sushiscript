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
struct FunctionType;

using TypeExprVisitor =
    sushi::util::DefineVisitor<TypeLit, ArrayType, MapType, FunctionType>;

// TODO: refactor after adding higher kinding type

struct TypeExpr {
    SUSHI_VISITABLE(TypeExprVisitor);

    virtual type::Type::Pointer ToType() const = 0;

    virtual ~TypeExpr() = default;
};

struct TypeLit : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    TypeLit(type::BuiltInAtom::Type type) : type(type) {}

    type::Type::Pointer ToType() const override {
        return type::BuiltInAtom::Make(type);
    }

    type::BuiltInAtom::Type type;
};

struct ArrayType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    ArrayType(type::BuiltInAtom::Type element) : element(std::move(element)) {}

    type::Type::Pointer ToType() const override {
        return type::Array::Make(element);
    }

    type::BuiltInAtom::Type element;
};

struct MapType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    MapType(type::BuiltInAtom::Type key, type::BuiltInAtom::Type value)
        : key(key), value(value) {}

    type::Type::Pointer ToType() const override {
        return type::Map::Make(key, value);
    }

    type::BuiltInAtom::Type key;
    type::BuiltInAtom::Type value;
};

struct FunctionType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    FunctionType(
        std::vector<std::unique_ptr<TypeExpr>> params,
        std::unique_ptr<TypeExpr> result)
        : params(std::move(params)), result(std::move(result)) {}

    type::Type::Pointer ToType() const override {
        type::Type::Pointer ret = result->ToType();
        std::vector<type::Type::Pointer> ps;
        for (auto& p : params) ps.push_back(p->ToType());
        return type::Function::Make(std::move(ps), std::move(ret));
    }

    std::vector<std::unique_ptr<TypeExpr>> params;
    std::unique_ptr<TypeExpr> result;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_TYPE_H_
