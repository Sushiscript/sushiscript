#ifndef SUSHI_AST_TYPE_H_
#define SUSHI_AST_TYPE_H_

#include "sushi/type-system/type.h"
#include "sushi/util/visitor.h"
#include <memory>
#include <utility>

namespace sushi {
namespace ast {

struct SimpleType;
struct ArrayType;
struct MapType;
struct FunctionType;

using TypeExprVisitor =
    sushi::util::DefineVisitor<SimpleType, ArrayType, MapType, FunctionType>;

// TODO: refactor after adding higher kinding type

struct TypeExpr {
    SUSHI_VISITABLE(TypeExprVisitor);

    virtual type::Type::Pointer ToType() const = 0;

    virtual ~TypeExpr() = default;
};

struct SimpleType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    SimpleType(type::Simple::Type type) : type(type) {}

    type::Type::Pointer ToType() const override {
        return type::Simple::Make(type);
    }

    type::Simple::Type type;
};

struct ArrayType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    ArrayType(type::Simple::Type element) : element(std::move(element)) {}

    type::Type::Pointer ToType() const override {
        return type::Array::Make(element);
    }

    type::Simple::Type element;
};

struct MapType : TypeExpr {
    SUSHI_ACCEPT_VISITOR_FROM(TypeExpr);

    MapType(type::Simple::Type key, type::Simple::Type value)
        : key(key), value(value) {}

    type::Type::Pointer ToType() const override {
        return type::Map::Make(key, value);
    }

    type::Simple::Type key;
    type::Simple::Type value;
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
