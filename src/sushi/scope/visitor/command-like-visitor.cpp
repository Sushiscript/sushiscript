#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void CommandLikeVisitor::Visit(const T &t)

VISIT(ast::FunctionCall, func_call) {
    ExpressionVisitor expression_visitor(environment, scope);
    for (auto &redir : func_call.redirs) {
        redir.external->AcceptVisitor(expression_visitor);
    }
    auto func_call_ptr = func_call.pipe_next.get();

    // This will recursively visit all the command-like
    if (func_call_ptr != nullptr) {
        CommandLikeVisitor command_like_visitor(environment, scope);
        func_call.pipe_next->AcceptVisitor(command_like_visitor);
        func_call_ptr = func_call_ptr->pipe_next.get();
    }
    for (auto &expr_ptr : func_call.parameters) {
        ExpressionVisitor expression_visitor(environment, scope);
        expr_ptr->AcceptVisitor(expression_visitor);
    }
    // insert info
    auto func_call_id = scope->LookUp(func_call.func.name);
    if (func_call_id == nullptr) {
        // TODO: Handle not defined error
        return;
    }
    environment.Insert(&func_call.func, scope);
}

VISIT(ast::Command, command) {
    ExpressionVisitor expression_visitor(environment, scope);
    for (auto &redir : command.redirs) {
        redir.external->AcceptVisitor(expression_visitor);
    }
    auto command_ptr = command.pipe_next.get();

    // This will recursively visit all the command-like
    if (command_ptr != nullptr) {
        CommandLikeVisitor command_like_visitor(environment, scope);
        command.pipe_next->AcceptVisitor(command_like_visitor);
        command_ptr = command_ptr->pipe_next.get();
    }

    LiteralVisitor lit_visitor(environment, scope);
    for (auto &param : command.parameters) {
        lit_visitor.ScopeInterpolation(param);
    }

    // cmd
    lit_visitor.ScopeInterpolation(command.cmd);
}

} // namespace scope
} // namespace sushi
