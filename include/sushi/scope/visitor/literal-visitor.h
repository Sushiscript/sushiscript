#ifndef SUSHI_SCOPE_LITERAL_VISITOR_H_
#define SUSHI_SCOPE_LITERAL_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope/environment.h"

namespace sushi {
namespace scope {

struct LiteralVisitor : public ast::LiteralVisitor::Const {
    Environment &environment;
    std::shared_ptr<Scope> &scope;

    LiteralVisitor(Environment &environment, std::shared_ptr<Scope> &scope)
        : environment(environment), scope(scope){};

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

    std::vector<Error> errs;
};

} // namespace scope
} // namespace sushi

#endif
