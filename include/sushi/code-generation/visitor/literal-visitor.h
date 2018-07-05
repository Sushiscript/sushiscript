#ifndef SUSHI_CODE_GEN_LITERAL_VISITOR_H_
#define SUSHI_CODE_GEN_LITERAL_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/code-generation/scope-manager.h"
#include <unordered_set>

namespace sushi {
namespace code_generation {

struct LiteralVisitor : public ast::LiteralVisitor::Const {
    std::string val;
    std::string code_before;
    std::string raw_id;
    std::unordered_set<std::string> new_ids;

    LiteralVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment &environment, const scope::Scope *scope)
        : scope_manager(scope_manager), environment(environment), scope(scope) {
    }

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

    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment &environment;
    const scope::Scope *scope;

    void TranslateInterpolation(const ast::InterpolatedString &inter_str);

    std::string GetTempName() {
        auto temp_name = scope_manager->GetNewTemp();
        new_ids.insert(temp_name);
        raw_id = temp_name;
        return temp_name;
    }
};

} // namespace code_generation
} // namespace sushi

#endif
