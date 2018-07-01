#include "sushi/scope.h"
#include "../include/literal-visitor.h"
#include "../include/expression-visitor.h"

namespace sushi {
namespace scope {
namespace test {

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
    ExpressionVisitor expression_visitor;
    for (auto &each_value : array_lit.value) {
        each_value->AcceptVisitor(expression_visitor);
    }
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::MapLit, map_lit) {
    ExpressionVisitor expression_visitor;
    for (auto &each_pair : map_lit.value) {
        each_pair.first->AcceptVisitor(expression_visitor);
        each_pair.second->AcceptVisitor(expression_visitor);
    }
    MergeVector(identifiers, expression_visitor.identifiers);
}

void LiteralVisitor::ScopeInterpolation(
    const ast::InterpolatedString &inter_str) {
    ExpressionVisitor expr_visitor;
    inter_str.Traverse(
        [](const std::string &) {},
        [this, &expr_visitor](const ast::Expression &expr) {
            expr.AcceptVisitor(expr_visitor);
        });
    MergeVector(identifiers, expr_visitor.identifiers);
}

}
} // namespace scope
} // namespace sushi
