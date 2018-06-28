#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void LiteralVisitor::Visit(const T &t)

VISIT(ast::IntLit, int_lit) {}
VISIT(ast::CharLit, char_lit) {}
VISIT(ast::BoolLit, bool_lit) {}
VISIT(ast::UnitLit, unit_lit) {}
VISIT(ast::FdLit, fd_lit) {}
VISIT(ast::StringLit, string_lit) {
    ScopeInterpolation(string_lit.value);
}
VISIT(ast::PathLit, path_lit) {
    ScopeInterpolation(path_lit.value);
}
VISIT(ast::RelPathLit, relPath_lit) {
    ScopeInterpolation(relPath_lit.value);
}
VISIT(ast::ArrayLit, array_lit) {
    ExpressionVisitor expression_visitor(environment, scope);
    for (auto &each_value : array_lit.value) {
        each_value->AcceptVisitor(expression_visitor);
    }
}

VISIT(ast::MapLit, map_lit) {
    ExpressionVisitor expression_visitor(environment, scope);
    for (auto &each_pair : map_lit.value) {
        each_pair.first->AcceptVisitor(expression_visitor);
        each_pair.second->AcceptVisitor(expression_visitor);
    }
}

void LiteralVisitor::ScopeInterpolation(
    const ast::InterpolatedString &inter_str) {
    inter_str.Traverse(
        [](const std::string &) {},
        [this](const ast::Expression &expr) {
            ExpressionVisitor expr_visitor(this->environment, this->scope);
            expr.AcceptVisitor(expr_visitor);
        });
}

} // namespace scope
} // namespace sushi
