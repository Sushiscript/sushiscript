#include "sushi/scope.h"
#include "../include/command-like-visitor.h"
#include "../include/expression-visitor.h"
#include "../include/literal-visitor.h"

namespace sushi {
namespace scope {
namespace test {

#define VISIT(T, t) void CommandLikeVisitor::Visit(const T &t)

VISIT(ast::FunctionCall, func_call) {
    for (auto &expr_ptr : func_call.parameters) {
        ExpressionVisitor expression_visitor;
        expr_ptr->AcceptVisitor(expression_visitor);
        MergeVector(identifiers, expression_visitor.identifiers);
    }

    ExpressionVisitor expression_visitor;
    for (auto &redir : func_call.redirs) {
        if (redir.external != nullptr)
            redir.external->AcceptVisitor(expression_visitor);
    }
    MergeVector(identifiers, expression_visitor.identifiers);


    auto func_call_ptr = func_call.pipe_next.get();
    // This will recursively visit all the command-like
    if (func_call_ptr != nullptr) {
        CommandLikeVisitor command_like_visitor;
        func_call.pipe_next->AcceptVisitor(command_like_visitor);
        MergeVector(identifiers, command_like_visitor.identifiers);
        func_call_ptr = func_call_ptr->pipe_next.get();
    }
}

VISIT(ast::Command, command) {
    // cmd
    LiteralVisitor lit_visitor;
    lit_visitor.ScopeInterpolation(command.cmd);

    for (auto &param : command.parameters) {
        lit_visitor.ScopeInterpolation(param);
    }

    MergeVector(identifiers, lit_visitor.identifiers);

    ExpressionVisitor expression_visitor;
    for (auto &redir : command.redirs) {
        if (redir.external != nullptr)
            redir.external->AcceptVisitor(expression_visitor);
    }
    MergeVector(identifiers, expression_visitor.identifiers);

    auto command_ptr = command.pipe_next.get();
    // This will recursively visit all the command-like
    if (command_ptr != nullptr) {
        CommandLikeVisitor command_like_visitor;
        command.pipe_next->AcceptVisitor(command_like_visitor);
        MergeVector(identifiers, command_like_visitor.identifiers);
        command_ptr = command_ptr->pipe_next.get();
    }
}

}
} // namespace scope
} // namespace sushi
