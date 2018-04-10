#ifndef SUSHI_TYPE_SYSTEM_TYPE_H_
#define SUSHI_TYPE_SYSTEM_TYPE_H_

#include "sushi/util/visitor.h"
#include <memory>
#include <vector>

namespace sushi {
namespace type {

struct BuiltInAtom;
struct Array;
struct Map;
struct Function;

using TypeVisitor =
    sushi::util::DefineVisitor<BuiltInAtom, Array, Map, Function>;

// highly simplified version of type that currently support only built-in types
struct Type {
    SUSHI_VISITABLE(TypeVisitor)
};

struct BuiltInAtom : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)

    enum class Type { kInt, kBool, kUnit, kFd, kExitCode, kPath, kString };

    BuiltInAtom(BuiltInAtom::Type type) : type(type) {}

    BuiltInAtom::Type type;
};

struct Array : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)
    std::unique_ptr<Type> element;
};

struct Map : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)
    BuiltInAtom::Type key;
    std::unique_ptr<Type> value;
};

struct Function : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type);
    std::vector<std::unique_ptr<Type>> params;
    std::unique_ptr<Type> result;
};

} // namespace type
} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_H_
