#ifndef SUSHI_SCOPE_TEST_LITERAL_VISITOR_H_
#define SUSHI_SCOPE_TEST_LITERAL_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope/environment.h"
#include "sushi/scope/error.h"

namespace sushi {
namespace scope {
namespace test {

struct LiteralVisitor : public ast::LiteralVisitor::Const {
    LiteralVisitor() {}

    SUSHI_VISITING(ast::IntLit, int_lit);
    SUSHI_VISITING(ast::CharLit, char_lit);
    SUSHI_VISITING(ast::BoolLit, bool_lit);
    SUSHI_VISITING(ast::UnitLit, unit_lit);
    SUSHI_VISITING(ast::FdLit, fd_lit);
    SUSHI_VISITING(ast::StringLit, string_lit);
    SUSHI_VISITING(ast::PathLit, path_lit);
    SUSHI_VISITING(ast::RelPathLit, relPath_lit);
    SUSHI_VISITING(ast::ArrayLit, array_lit);
    SUSHI_VISITING(ast::MapLit, map_lit);

    void ScopeInterpolation(const ast ::InterpolatedString &inter_str);

    std::vector<const ast::Program*> programs;
    std::vector<const ast::Identifier*> identifiers;
};

}
} // namespace scope
} // namespace sushi

#endif
