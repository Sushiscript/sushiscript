#ifndef SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_
#define SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope.h"
#include "./scope-manager.h"

namespace sushi {

struct CodeGenCmdLikeVisitor : public ast::CommandLikeVisitor::Const {
    std::string val;
    std::string code_before;

    CodeGenCmdLikeVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const Environment & environment,
        const Scope * scope,
        bool is_left_value = false
        ) : is_left_value(is_left_value),
            scope_manager(scope_manager),
            environment(environment),
            scope(scope) {}

    SUSHI_VISITING(ast::FunctionCall, func_call);
    SUSHI_VISITING(ast::Command, command);

  protected:
    bool is_left_value;
    std::shared_ptr<ScopeManager> scope_manager;
    const Environment & environment;
    const Scope * scope;
};

} // namespace sushi

#endif
