#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void TypeVisitor::Visit(const T &t)

VISIT(type::BuiltInAtom, built_in_atom) {}
VISIT(type::Array, array) {}
VISIT(type::Map, map) {}
VISIT(type::Function, function) {}

} // namespace scope
} // namespace sushi
