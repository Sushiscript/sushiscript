#ifndef SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_
#define SUSHI_CODE_GEN_CMDLIKE_VISITOR_H_

#include "./expr-visitor.h"
#include "./scope-manager.h"
#include "./type-visitor.h"
#include "sushi/ast.h"
#include "sushi/scope.h"

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
        const scope::Environment &environment, const scope::Scope *scope,
        bool is_first_cmd_like = true)
        : scope_manager(scope_manager), environment(environment), scope(scope),
          is_first_cmd_like(is_first_cmd_like) {}

    using ST = TypeVisitor::SimplifiedType;

    SUSHI_VISITING(ast::FunctionCall, func_call);
    SUSHI_VISITING(ast::Command, command);

  protected:
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment &environment;
    const scope::Scope *scope;
    bool is_first_cmd_like;

    std::string cmd_like_str;

    struct FuncOrCmdVisitor : ast::CommandLikeVisitor::Const {
        enum class FuncOrCmd { kFunc, kCmd };

        FuncOrCmd f_or_c;

        SUSHI_VISITING(ast::FunctionCall, func_call) {
            f_or_c = FuncOrCmd::kFunc;
        }

        SUSHI_VISITING(ast::Command, command) {
            f_or_c = FuncOrCmd::kCmd;
        }
    };

    void TranslateFinalCmdLike(
        ast::CommandLike *cmd_like_ptr, const std::string &temp_name) {
        FuncOrCmdVisitor visitor;
        cmd_like_ptr->AcceptVisitor(visitor);

        constexpr char kVarDefTemplate[] = "local %1%=%2%";
        constexpr char kVarDefArrayTemplate[] = "local -a %1%=(%2%)";
        constexpr char kVarDefMapTemplate[] =
            "local -A %1%=(); eval \"%1%=(%2%)\"";
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
            default:
                assert(false && "Type is not supposed to be here");
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
                code_before += (boost::format(kVarDefTemplate) % temp_name %
                                "${_sushi_func_ret_}")
                                   .str();
                break;
            case ST::kArray:
                code_before += (boost::format(kVarDefArrayTemplate) %
                                temp_name % "\"${_sushi_func_ret_[@]}\"")
                                   .str();
                break;
            case ST::kMap:
                auto temp = scope_manager->GetNewTemp();
                new_ids.insert(temp);
                code_before += (boost::format(kMapVarCodeBeforeTemplate) %
                                temp % temp_name)
                                   .str();
                code_before += '\n';
                code_before += (boost::format(kVarDefMapTemplate) % temp_name %
                                ("\"${" + temp + "}\""))
                                   .str();
                break;
            }

            val = "${" + temp_name + '}';
        } else if (visitor.f_or_c == FOC::kCmd) {
            code_before +=
                (boost::format(kVarDefTemplate) % temp_name % "$?").str();
            val = "${" + temp_name + '}';
        }
    }
};

} // namespace code_generation
} // namespace sushi

#endif
