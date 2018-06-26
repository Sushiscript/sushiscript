#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void LiteralVisitor::Visit(const T &t)

VISIT(ast::IntLit, int_lit) {}
VISIT(ast::CharLit, char_lit) {}
VISIT(ast::BoolLit, bool_lit) {}
VISIT(ast::UnitLit, unit_lit) {}
VISIT(ast::FdLit, fd_lit) {}
VISIT(ast::StringLit, string_lit) {}
VISIT(ast::PathLit, path_lit) {}
VISIT(ast::RelPathLit, relPath_lit) {}
VISIT(ast::ArrayLit, array_lit) {
    ExpressionVisitor expression_visitor (environment, scope);
    for (auto & each_value : array_lit.value) {
        each_value->AcceptVisitor(expression_visitor);
    }
}

VISIT(ast::MapLit, map_lit) {
    ExpressionVisitor expression_visitor (environment, scope);
    for (auto & each_pair : map_lit.value) {
        each_pair.first->AcceptVisitor(expression_visitor);
        each_pair.second->AcceptVisitor(expression_visitor);
    }
}

} // namespace scope
} // namespace sushi
