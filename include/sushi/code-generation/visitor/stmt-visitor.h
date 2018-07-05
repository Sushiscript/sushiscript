#ifndef SUSHI_CODE_GEN_STMT_VISITOR_H_
#define SUSHI_CODE_GEN_STMT_VISITOR_H_

#include "sushi/code-generation/scope-manager.h"
#include "sushi/code-generation/visitor/type-visitor.h"
#include <unordered_set>

namespace sushi {
namespace code_generation {

constexpr char kAssignTemplate[] = "%1%=%2%";
constexpr char kAssignArrayTemplate[] = "%1%=(%2%)";
constexpr char kAssignMapTemplate[] = "eval \"%1%=(%2%)\"";

constexpr char kVarDefTemplate[] = "local %1%=%2%";
constexpr char kVarDefArrayTemplate[] = "local -a %1%=(%2%)";
constexpr char kVarDefMapTemplate[] = "local -A %1%=(); eval \"%1%=(%2%)\"";
constexpr char kVarDefExpoTemplate[] = "declare -x%1% %2%=%3%";

constexpr char kFuncDefTemplate[] =
    R"(local %1%=%1%
%1% () {
%2%
})";
constexpr char kFuncDefExpoTemplate[] =
    R"(local %1%=%1%
%1% () {
%2%
}
export -f %1%)";

constexpr char kReturnStmtNotBoolTemplate[] = "_sushi_func_ret_=%1%; return 0";
constexpr char kReturnStmtBoolTemplate[] =
    R"(_sushi_func_ret_=%1%
if [[ _sushi_func_ret_ -ne 0 ]]; then
    return 0
else
    return 1
fi)";
constexpr char kReturnStmtArrayTemplate[] = "_sushi_func_ret_=(%1%); return 0";
constexpr char kReturnStmtMapTemplate[] =
    R"foo(eval "_sushi_func_map_ret_=(%1%)"; return 0)foo";

constexpr char kIfStmtPartTemplate[] = "if [[ %1% -ne 0 ]]; then\n%2%\nfi";
constexpr char kIfStmtFullTemplate[] =
    "if [[ %1% -ne 0 ]]; then\n%2%\nelse\n%3%\nfi";
constexpr char kIfStmtExitCodePartTemplate[] =
    "if [[ %1% -eq 0 ]]; then\n%2%\nfi";
constexpr char kIfStmtExitCodeFullTemplate[] =
    "if [[ %1% -eq 0 ]]; then\n%2%\nelse\n%3%\nfi";

constexpr char kForStmtIterTemplate[] = "for %1% in \"%2%\"; do\n%3%\ndone";
constexpr char kForStmtWhileTemplate[] = "while [[ %1% -ne 0 ]]; do\n%2%\ndone";
constexpr char kForStmtWhileExitCodeTemplate[] =
    "while [[ %1% -eq 0 ]]; do\n%2%\ndone";

inline std::string ExitCodeExprToBool(const std::string &str) {
    return "$((! " + str + "))";
}

inline std::string ExitCodeExprToInt(const std::string &str) {
    return str;
}

inline std::string RelPathExprToPath(const std::string &str) {
    return "\"$(pwd)/" + str + "\"";
}

struct StmtVisitor : public ast::StatementVisitor::Const {
    std::string code;
    const scope::Environment &environment;
    const ast::Program &program;
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Scope *scope;

    std::unordered_set<std::string> new_ids;

    using ST = TypeVisitor::SimplifiedType;

    StmtVisitor(
        const scope::Environment &environment, const ast::Program &program,
        std::shared_ptr<ScopeManager> scope_manager)
        : environment(environment), program(program),
          scope_manager(scope_manager) {
        scope = environment.LookUp(&program);
    }

    SUSHI_VISITING(ast::Assignment, assignment);
    SUSHI_VISITING(ast::Expression, expression);
    SUSHI_VISITING(ast::VariableDef, var_def);
    SUSHI_VISITING(ast::FunctionDef, func_def);
    SUSHI_VISITING(ast::ReturnStmt, return_stmt);
    SUSHI_VISITING(ast::IfStmt, if_stmt);

    SUSHI_VISITING(ast::SwitchStmt, switch_stmt);
    SUSHI_VISITING(ast::ForStmt, for_stmt);
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt);
};

} // namespace code_generation
} // namespace sushi

#endif
