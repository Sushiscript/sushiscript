#ifndef SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_
#define SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_

#include "sushi/ast.h"
#include "sushi/scope.h"
#include "./scope-manager.h"
#include "./type-visitor.h"

#include <unordered_set>

namespace sushi {
namespace code_generation {

struct CmdLikeVisitor : public ast::CommandLikeVisitor::Const {
    std::string val;
    std::string code_before;
    std::string raw_id;
    bool redir_to_here = false;

    std::unordered_set<std::string> new_ids;

    CmdLikeVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment & environment,
        const scope::Scope * scope,
        bool is_first_cmd_like = true
        ) : scope_manager(scope_manager),
            environment(environment),
            scope(scope),
            is_first_cmd_like(is_first_cmd_like) {}

    using ST = TypeVisitor::SimplifiedType;

    SUSHI_VISITING(ast::FunctionCall, func_call) {
        auto new_name = scope_manager->FindNewName(func_call.func.name, scope);

        cmd_like_str = "${" + new_name + '}';

        // parameters
        for (auto &expr : func_call.parameters) {
            ExprVisitor expr_visitor(scope_manager, environment, scope);
            expr->AcceptVisitor(expr_visitor);
            new_ids.merge(expr_visitor.new_ids);

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
                    dir_str = ">>"; break;
                } else {
                    dir_str = ">"; break;
                }
            }

            ExprVisitor expr_visitor(scope_manager, environment, scope);
            redir.external->AcceptVisitor(expr_visitor);
            new_ids.merge(expr_visitor.new_ids);
            code_before += expr_visitor.code_before + '\n';

            auto type = environment.LookUp(redir.external.get());
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);

            std::string redir_item;

            using ST = TypeVisitor::SimplifiedType;
            switch (type_visitor.type) {
            case ST::kPath:
            case ST::kRelPath:
                redir_item = (boost::format(redir_template) % me_str
                                                            % dir_str
                                                            % expr_visitor.val).str();
                break;
            case ST::kFd:
                redir_item = (boost::format(redir_template) % me_str
                                                            % dir_str
                                                            % ('&' + expr_visitor.val)).str();
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
            CmdLikeVisitor cmd_like_visitor(scope_manager, environment, scope, false);
            next_ptr->AcceptVisitor(cmd_like_visitor);
            code_before += cmd_like_visitor.code_before + '\n';
            new_ids.merge(cmd_like_visitor.new_ids);

            cmd_like_str += " | " + cmd_like_visitor.cmd_like_str;

            if (cmd_like_visitor.redir_to_here) {
                cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
            }

            next_ptr = next_ptr->pipe_next.get();
            if (next_ptr) final_ptr = next_ptr;
            else if (cmd_like_visitor.redir_to_here) final_to_here = true;
        }

        auto temp_name = scope_manager->GetNewTemp();
        new_ids.insert(temp_name);
        raw_id = temp_name;

        // call
        if (final_to_here) {
            // if has redir to here, use a temp var to store stdout
            code_before += (boost::format("local %1%=%2%") % temp_name % cmd_like_str).str();
            val = "${" + temp_name + '}';
        } else {
            // if no redir to here, use a temp var to copy function return value
            TranslateFinalCmdLike(final_ptr, temp_name);
        }
    }
    SUSHI_VISITING(ast::Command, command) {
        LiteralVisitor cmd_visitor(scope_manager, environment, scope);
        cmd_visitor.TranslateInterpolation(command.cmd);
        new_ids.merge(cmd_visitor.new_ids);
        code_before += cmd_visitor.code_before + '\n';

        cmd_like_str = cmd_visitor.val;

        //  parameters
        for (auto &inter : command.parameters) {
            LiteralVisitor inter_visitor(scope_manager, environment, scope);
            inter_visitor.TranslateInterpolation(inter);
            new_ids.merge(inter_visitor.new_ids);
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
                    dir_str = ">>"; break;
                } else {
                    dir_str = ">"; break;
                }
            }

            ExprVisitor expr_visitor(scope_manager, environment, scope);
            redir.external->AcceptVisitor(expr_visitor);
            new_ids.merge(expr_visitor.new_ids);
            code_before += expr_visitor.code_before + '\n';

            auto type = environment.LookUp(redir.external.get());
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);

            std::string redir_item;

            using ST = TypeVisitor::SimplifiedType;
            switch (type_visitor.type) {
            case ST::kPath:
            case ST::kRelPath:
                redir_item = (boost::format(redir_template) % me_str
                                                            % dir_str
                                                            % expr_visitor.val).str();
                break;
            case ST::kFd:
                redir_item = (boost::format(redir_template) % me_str
                                                            % dir_str
                                                            % ('&' + expr_visitor.val)).str();
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
            CmdLikeVisitor cmd_like_visitor(scope_manager, environment, scope, false);
            next_ptr->AcceptVisitor(cmd_like_visitor);
            code_before += cmd_like_visitor.code_before + '\n';
            new_ids.merge(cmd_like_visitor.new_ids);

            cmd_like_str += " | " + cmd_like_visitor.cmd_like_str;

            if (cmd_like_visitor.redir_to_here) {
                cmd_like_str = (boost::format("$(%1%)") % cmd_like_str).str();
            }

            next_ptr = next_ptr->pipe_next.get();
            if (next_ptr) final_ptr = next_ptr;
            else if (cmd_like_visitor.redir_to_here) final_to_here = true;
        }

        auto temp_name = scope_manager->GetNewTemp();
        new_ids.insert(temp_name);
        raw_id = temp_name;

        // call
        if (final_ptr) {
            // if has redir to here, use a temp var to store stdout
            code_before += (boost::format("%1%=%2%") % temp_name % cmd_like_str).str();
            val = "${" + temp_name + '}';
        } else {
            // if no redir to here, use a temp var to store $?
            TranslateFinalCmdLike(final_ptr, temp_name);
        }
    }

  protected:
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment & environment;
    const scope::Scope * scope;
    bool is_first_cmd_like;

    std::string cmd_like_str;

    struct FuncOrCmdVisitor : ast::CommandLikeVisitor::Const {
        enum class FuncOrCmd {
            kFunc,
            kCmd
        };

        FuncOrCmd f_or_c;

        SUSHI_VISITING(ast::FunctionCall, func_call) {
            f_or_c = FuncOrCmd::kFunc;
        }

        SUSHI_VISITING(ast::Command, command) {
            f_or_c = FuncOrCmd::kCmd;
        }
    };

    void TranslateFinalCmdLike(ast::CommandLike *cmd_like_ptr, const std::string & temp_name) {
        FuncOrCmdVisitor visitor;
        cmd_like_ptr->AcceptVisitor(visitor);

        constexpr char kVarDefTemplate[] = "local %1%=%2%";
        constexpr char kVarDefArrayTemplate[] = "local -a %1%=(%2%)";
        constexpr char kVarDefMapTemplate[] = "local -A %1%=(); eval \"%1%=(%2%)\"";
        constexpr char kMapVarCodeBeforeTemplate[] =
R"(local %1%=`declare -p %2%`
%1%=${%1%#*=}
%1%=${%1%:1:-1}
)";
        using FOC = FuncOrCmdVisitor::FuncOrCmd;
        if (visitor.f_or_c == FOC::kFunc) {

            auto type = environment.LookUp(cmd_like_ptr);
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);
            switch (type_visitor.type) {
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
                code_before += (boost::format(kVarDefTemplate) % temp_name
                                                               % "${_sushi_func_ret_}").str();
                break;
            case ST::kArray:
                code_before += (boost::format(kVarDefArrayTemplate) % temp_name
                                                                    % "\"${_sushi_func_ret_[@]}\"").str();
                break;
            case ST::kMap:
                auto temp = scope_manager->GetNewTemp();
                new_ids.insert(temp);
                code_before += (boost::format(kMapVarCodeBeforeTemplate) % temp % temp_name).str();
                code_before += '\n';
                code_before += (boost::format(kVarDefMapTemplate) % temp_name
                                                                  % ("\"${" + temp + "}\"")).str();
                break;
            }

            val = "${" + temp_name + '}';
        } else if (visitor.f_or_c == FOC::kCmd) {
            code_before += (boost::format(kVarDefTemplate) % temp_name
                                                           % "$?").str();
            val = "${" + temp_name + '}';
        }
    }
};

} // namespace code_generation
} // namespace sushi

#endif
