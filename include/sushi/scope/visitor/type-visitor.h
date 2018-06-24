#ifndef SUSHI_SCOPE_TYPE_VISITOR_H_
#define SUSHI_SCOPE_TYPE_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope/environment.h"

namespace sushi {
namespace scope {

struct TypeExprVisitor : public ast::TypeExprVisitor::Const {
    Environment & environment;

    TypeExprVisitor(Environment & environment) : environment(environment) {};

    SUSHI_VISITING(ast::TypeLit, type_lit) {
    }
    SUSHI_VISITING(ast::ArrayType, array_type) {
    }
    SUSHI_VISITING(ast::MapType, map_type) {
    }
    SUSHI_VISITING(ast::FunctionType, function_type) {
    }
};

struct TypeVisitor : public type::TypeVisitor::Const {
    Environment & environment;

    TypeVisitor(Environment & environment) : environment(environment) {};

    SUSHI_VISITING(type::BuiltInAtom, built_in_atom) {
    }
    SUSHI_VISITING(type::Array, array) {
    }
    SUSHI_VISITING(type::Map, map) {
    }
    SUSHI_VISITING(type::Function, function) {
    }
};

} // namespace scope 
} // namespace sushi

#endif
