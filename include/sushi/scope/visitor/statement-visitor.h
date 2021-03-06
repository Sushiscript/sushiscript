#ifndef SUSHI_SCOPE_STATEMENT_VISITOR_H_
#define SUSHI_SCOPE_STATEMENT_VISITOR_H_

#include "expression-visitor.h"
#include "sushi/ast.h"
#include "sushi/scope/environment.h"

namespace sushi {
namespace scope {

struct StatementVisitor : public ast::StatementVisitor::Const {
    Environment &environment;
    std::shared_ptr<Scope> &scope;

    StatementVisitor(Environment &environment, std::shared_ptr<Scope> &scope)
        : environment(environment), scope(scope){};

    SUSHI_VISITING(ast::Assignment, assignment);
    SUSHI_VISITING(ast::VariableDef, var_def);
    SUSHI_VISITING(ast::FunctionDef, func_def);
    SUSHI_VISITING(ast::IfStmt, if_stmt);
    SUSHI_VISITING(ast::ReturnStmt, return_stmt);
    SUSHI_VISITING(ast::SwitchStmt, switch_stmt);
    SUSHI_VISITING(ast::ForStmt, for_stmt);
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt);
    SUSHI_VISITING(ast::Expression, expression);

    std::vector<Error> errs;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_STATEMENT_VISITOR_H_
