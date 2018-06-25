#include "sushi/code-generation/cmdlike-visitor.h"
#include "sushi/code-generation/util.h"

namespace sushi {
namespace code_generation {

#define CMDLIKE_VISITING_IMPL(T, t) void CmdLikeVisitor::Visit(const T &t)

CMDLIKE_VISITING_IMPL(ast::FunctionCall, func_call) {
    auto new_name = scope_manager->FindNewName(func_call.func.name, scope);

    cmd_like_str = "${" + new_name + '}';

    // parameters
    for (auto &expr : func_call.parameters) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        expr->AcceptVisitor(expr_visitor);
        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);

        code_before += expr_visitor.code_before + '\n';
        // use raw_id("variable name") to pass parameter
        cmd_like_str += ' ' + expr_visitor.raw_id;
    }

    using V = ast::FdLit::Value;
    using DIR = ast::Redirection::Direction;
    // redirections
    // 1: fd, 2: </>/>>, 3: fd/Path
    std::string redir_str;
    constexpr char redir_template[] = "%1%%2%%3%";
    for (auto &redir : func_call.redirs) {
        if (redir.direction == DIR::kOut && !redir.external) {
            // to here
            redir_to_here = true;
            continue;
        }

        std::string me_str;
        switch (redir.me) {
        case V::kStdin: me_str = "0"; break;
        case V::kStdout: me_str = "1"; break;
        case V::kStderr: me_str = "2"; break;
        }

        std::string dir_str;
        switch (redir.direction) {
        case DIR::kIn: dir_str = "<"; break;
        case DIR::kOut:
            if (redir.append) {
                dir_str = ">>";
                break;
            } else {
                dir_str = ">";
                break;
            }
        }

        ExprVisitor expr_visitor(scope_manager, environment, scope);
        redir.external->AcceptVisitor(expr_visitor);
        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);
        code_before += expr_visitor.code_before + '\n';

        auto type = environment.LookUp(redir.external.get());
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        std::string redir_item;

        using ST = TypeVisitor::SimplifiedType;
        switch (type_visitor.type) {
        case ST::kPath:
        case ST::kRelPath:
            redir_item = (boost::format(redir_template) % me_str % dir_str %
                          expr_visitor.val)
                             .str();
            break;
        case ST::kFd:
            redir_item = (boost::format(redir_template) % me_str % dir_str %
                          ('&' + expr_visitor.val))
                             .str();
            break;
        default:
            assert(false && "Type is not supposed to be here");
        }
        redir_str += ' ' + redir_item;
    }
    cmd_like_str += redir_str;
    cmd_like_str = (boost::format("eval \"%1%\"") % cmd_like_str).str();

    if (redir_to_here) {
        cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
    }

    if (!is_first_cmd_like) return;

    auto next_ptr = func_call.pipe_next.get();
    auto final_ptr = next_ptr;
    bool final_to_here = false;
    while (next_ptr) {
        CmdLikeVisitor cmd_like_visitor(
            scope_manager, environment, scope, false);
        next_ptr->AcceptVisitor(cmd_like_visitor);
        code_before += cmd_like_visitor.code_before + '\n';
        // new_ids.merge(cmd_like_visitor.new_ids);
        MergeSets(new_ids, cmd_like_visitor.new_ids);

        cmd_like_str += " | " + cmd_like_visitor.cmd_like_str;

        if (cmd_like_visitor.redir_to_here) {
            cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
        }

        next_ptr = next_ptr->pipe_next.get();
        if (next_ptr)
            final_ptr = next_ptr;
        else if (cmd_like_visitor.redir_to_here)
            final_to_here = true;
    }

    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    // call
    if (final_to_here) {
        // if has redir to here, use a temp var to store stdout
        code_before +=
            (boost::format("local %1%=%2%") % temp_name % cmd_like_str).str();
        val = "${" + temp_name + '}';
    } else {
        // if no redir to here, use a temp var to copy function return value
        TranslateFinalCmdLike(final_ptr, temp_name);
    }
}

CMDLIKE_VISITING_IMPL(ast::Command, command) {
    LiteralVisitor cmd_visitor(scope_manager, environment, scope);
    cmd_visitor.TranslateInterpolation(command.cmd);
    // new_ids.merge(cmd_visitor.new_ids);
    MergeSets(new_ids, cmd_visitor.new_ids);
    code_before += cmd_visitor.code_before + '\n';

    cmd_like_str = cmd_visitor.val;

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

    using V = ast::FdLit::Value;
    using DIR = ast::Redirection::Direction;
    // redirections
    // 1: fd, 2: </>/>>, 3: fd/Path
    std::string redir_str;
    constexpr char redir_template[] = "%1%%2%%3%";
    for (auto &redir : command.redirs) {
        if (redir.direction == DIR::kOut && !redir.external) {
            // to here
            redir_to_here = true;
            continue;
        }

        std::string me_str;
        switch (redir.me) {
        case V::kStdin: me_str = "0"; break;
        case V::kStdout: me_str = "1"; break;
        case V::kStderr: me_str = "2"; break;
        }

        std::string dir_str;
        switch (redir.direction) {
        case DIR::kIn: dir_str = "<"; break;
        case DIR::kOut:
            if (redir.append) {
                dir_str = ">>";
                break;
            } else {
                dir_str = ">";
                break;
            }
        }

        ExprVisitor expr_visitor(scope_manager, environment, scope);
        redir.external->AcceptVisitor(expr_visitor);
        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);
        code_before += expr_visitor.code_before + '\n';

        auto type = environment.LookUp(redir.external.get());
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        std::string redir_item;

        using ST = TypeVisitor::SimplifiedType;
        switch (type_visitor.type) {
        case ST::kPath:
        case ST::kRelPath:
            redir_item = (boost::format(redir_template) % me_str % dir_str %
                          expr_visitor.val)
                             .str();
            break;
        case ST::kFd:
            redir_item = (boost::format(redir_template) % me_str % dir_str %
                          ('&' + expr_visitor.val))
                             .str();
            break;
        default:
            assert(false && "Type is not supposed to be here");
        }
        redir_str += ' ' + redir_item;
    }
    cmd_like_str += redir_str;

    if (redir_to_here) {
        cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
    }

    if (!is_first_cmd_like) return;

    auto next_ptr = command.pipe_next.get();
    auto final_ptr = next_ptr;
    bool final_to_here = false;
    while (next_ptr) {
        CmdLikeVisitor cmd_like_visitor(
            scope_manager, environment, scope, false);
        next_ptr->AcceptVisitor(cmd_like_visitor);
        code_before += cmd_like_visitor.code_before + '\n';
        // new_ids.merge(cmd_like_visitor.new_ids);
        MergeSets(new_ids, cmd_like_visitor.new_ids);

        cmd_like_str += " | " + cmd_like_visitor.cmd_like_str;

        if (cmd_like_visitor.redir_to_here) {
            cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
        }

        next_ptr = next_ptr->pipe_next.get();
        if (next_ptr)
            final_ptr = next_ptr;
        else if (cmd_like_visitor.redir_to_here)
            final_to_here = true;
    }

    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    // call
    if (final_ptr) {
        // if has redir to here, use a temp var to store stdout
        code_before +=
            (boost::format("%1%=%2%") % temp_name % cmd_like_str).str();
        val = "${" + temp_name + '}';
    } else {
        // if no redir to here, use a temp var to store $?
        TranslateFinalCmdLike(final_ptr, temp_name);
    }
}

} // namespace code_generation
} // namespace sushi
