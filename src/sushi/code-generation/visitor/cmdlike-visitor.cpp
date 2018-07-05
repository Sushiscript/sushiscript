#include "sushi/code-generation.h"

namespace sushi {
namespace code_generation {

#define CMDLIKE_VISITING_IMPL(T, t) void CmdLikeVisitor::Visit(const T &t)

CMDLIKE_VISITING_IMPL(ast::FunctionCall, func_call) {
    auto func_scope = scope->LookUp(func_call.func.name)->defined_scope;
    auto new_name = scope_manager->FindNewName(func_call.func.name, func_scope);

    cmd_like_str = "${" + new_name + '}';

    // parameters
    for (auto &expr : func_call.parameters) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        expr->AcceptVisitor(expr_visitor);
        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);

        code_before += expr_visitor.code_before + '\n';
        // <del>use raw_id("variable name") to pass parameter</del>
        // pass simple type by val
        // pass array/map by name
        auto type = GetType(environment, expr.get());
        switch (type) {
        case ST::kInt:
        case ST::kBool:
        case ST::kUnit:
        case ST::kFd:
        case ST::kExitCode:
        case ST::kPath:
        case ST::kRelPath:
        case ST::kString:
        case ST::kChar:
        case ST::kFunc:
            cmd_like_str += ' ' + expr_visitor.val;
        case ST::kMap:
        case ST::kArray:
            cmd_like_str += ' ' + expr_visitor.raw_id;
            break;
        }
    }

    auto redir_res = ProcessRedirs(func_call);
    if (redir_res.final_to_here == false && redir_res.final_ptr == nullptr)
        return;
    ProcessCall(func_call, redir_res);
}

CMDLIKE_VISITING_IMPL(ast::Command, command) {
    LiteralVisitor cmd_visitor(scope_manager, environment, scope);
    cmd_visitor.TranslateInterpolation(command.cmd);
    // new_ids.merge(cmd_visitor.new_ids);
    MergeSets(new_ids, cmd_visitor.new_ids);
    code_before += cmd_visitor.code_before + '\n';

    cmd_like_str = cmd_visitor.val;

    MergeSets(new_ids, cmd_visitor.new_ids);

    //  parameters
    for (auto &inter : command.parameters) {
        LiteralVisitor inter_visitor(scope_manager, environment, scope);
        inter_visitor.TranslateInterpolation(inter);
        // new_ids.merge(inter_visitor.new_ids);
        MergeSets(new_ids, inter_visitor.new_ids);
        code_before += inter_visitor.code_before + '\n';

        // cmd doesn't use raw_id
        cmd_like_str += ' ' + inter_visitor.val;
    }

    auto redir_res = ProcessRedirs(command);
    if (redir_res.final_to_here == false && redir_res.final_ptr == nullptr)
        return;
    ProcessCall(command, redir_res);
}

} // namespace code_generation
} // namespace sushi
