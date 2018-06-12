#ifndef SUSHI_CODE_GEN_LITERAL_VISITOR_H_
#define SUSHI_CODE_GEN_LITERAL_VISITOR_H_

#include "sushi/ast.h"

namespace sushi {

struct CodeGenLiteralVisitor : public ast::LiteralVisitor::Const {
    std::string val;
    std::string code_before;

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
};

} // namespace sushi

#endif
