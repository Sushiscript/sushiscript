#ifndef SUSHI_CODE_GEN_STMT_VISITOR_H_
#define SUSHI_CODE_GEN_STMT_VISITOR_H_

#include "./code-generation.h"
#include "./expr-visitor.h"
#include "./type-visitor.h"
#include "./util.h"
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
    R"(%1% () {
%2%
}
local %1%=%1%)";
constexpr char kFuncDefExpoTemplate[] =
    R"(%1% () {
%2%
}
export -f %1%
local %1%=%1%)";

constexpr char kReturnStmtNotBoolTemplate[] = "_sushi_func_ret_=%1%";
constexpr char kReturnStmtBoolTemplate[] =
    R"(_sushi_func_ret_=%1%
if [[ _sushi_func_ret_ -ne 0 ]]; then
    return 0
else
    return 1
fi)";
constexpr char kReturnStmtArrayTemplate[] = "_sushi_func_ret_=(%1%)";
constexpr char kReturnStmtMapTemplate[] =
    R"foo(eval "_sushi_func_map_ret_=(%1%)")foo";

constexpr char kIfStmtPartTemplate[] = "if [[ %1% -ne 0 ]]; then\n%2%\nfi";
constexpr char kIfStmtFullTemplate[] =
    "if [[ %1% -ne 0 ]]; then\n%2%\nelse\n%3%\nfi";
constexpr char kIfStmtExitCodePartTemplate[] =
    "if [[ %1% -eq 0 ]]; then\n%2%\nfi";
constexpr char kIfStmtExitCodeFullTemplate[] =
    "if [[ %1% -eq 0 ]]; then\n%2%\nelse\n%3%\nfi";

constexpr char kForStmtIterTemplate[] = "for %1% in %2%; do\n%3%\ndone";
constexpr char kForStmtWhileTemplate[] = "while [[ %1% -ne 0 ]]; do\n%2%\ndone";
constexpr char kForStmtWhileExitCodeTemplate[] =
    "while [[ %1% -eq 0 ]]; do\n%2%\ndone";

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

    SUSHI_VISITING(ast::Assignment, assignment) {
        ExprVisitor lvalue_expr_visitor(
            scope_manager, environment, scope, true);
        assignment.lvalue->AcceptVisitor(lvalue_expr_visitor);
        ExprVisitor rvalue_expr_visitor(
            scope_manager, environment, scope, false);
        assignment.value->AcceptVisitor(rvalue_expr_visitor);

        // new_ids.merge(lvalue_expr_visitor.new_ids);
        MergeSets(new_ids, lvalue_expr_visitor.new_ids);
        // new_ids.merge(rvalue_expr_visitor.new_ids);
        MergeSets(new_ids, rvalue_expr_visitor.new_ids);

        auto type = environment.LookUp(assignment.value.get());

        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        code += lvalue_expr_visitor.code_before + '\n' +
                rvalue_expr_visitor.code_before + '\n';
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
            code += (boost::format(kAssignTemplate) % lvalue_expr_visitor.val %
                     rvalue_expr_visitor.val)
                        .str();
            break;
        case ST::kMap:
            code += (boost::format(kAssignMapTemplate) %
                     lvalue_expr_visitor.val % rvalue_expr_visitor.val)
                        .str();
            break;
        case ST::kArray:
            code += (boost::format(kAssignArrayTemplate) %
                     lvalue_expr_visitor.val % rvalue_expr_visitor.val)
                        .str();
            break;
        }
    }
    SUSHI_VISITING(ast::Expression, expression) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        expression.AcceptVisitor(expr_visitor);

        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);

        code += expr_visitor.code_before;
        code += '\n' + expr_visitor.val;
    }
    SUSHI_VISITING(ast::VariableDef, var_def) {
        const scope::Scope *scope = environment.LookUp(&program);
        auto new_name = scope_manager->GetNewName(var_def.name, scope);
        TypeExprVisitor type_expr_visitor;
        var_def.type->AcceptVisitor(type_expr_visitor);
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        var_def.value->AcceptVisitor(expr_visitor);

        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);
        if (!var_def.is_export) new_ids.insert(new_name);

        code += expr_visitor.code_before + '\n';

        switch (type_expr_visitor.type) {
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
            if (var_def.is_export) {
                code += (boost::format(kVarDefExpoTemplate) % "" % new_name %
                         expr_visitor.val)
                            .str();
            } else {
                code += (boost::format(kVarDefTemplate) % new_name %
                         expr_visitor.val)
                            .str();
            }
            break;
        case ST::kMap:
            if (var_def.is_export) {
                code += (boost::format(kVarDefExpoTemplate) % "a" % new_name %
                         expr_visitor.val)
                            .str();
            } else {
                code += (boost::format(kVarDefMapTemplate) % new_name %
                         expr_visitor.val)
                            .str();
            }
            break;
        case ST::kArray:
            if (var_def.is_export) {
                code += (boost::format(kVarDefExpoTemplate) % "A" % new_name %
                         expr_visitor.val)
                            .str();
            } else {
                code += (boost::format(kVarDefArrayTemplate) % new_name %
                         expr_visitor.val)
                            .str();
            }
            break;
        }
    }
    SUSHI_VISITING(ast::FunctionDef, func_def) {
        const scope::Scope *scope = environment.LookUp(&program);
        TypeExprVisitor ret_type_visitor;
        func_def.ret_type->AcceptVisitor(ret_type_visitor);
        auto new_name = scope_manager->GetNewName(func_def.name, scope);

        if (!func_def.is_export) new_ids.insert(new_name);

        // Params assignment
        std::string param_assign_part;
        constexpr char kParamAssignTemplate[] = "local -n %1%=%2%";
        for (int i = 0; i < func_def.params.size(); ++i) {
            auto &param = func_def.params[i];
            std::string line;
            line = (boost::format(kParamAssignTemplate) % param.name %
                    std::to_string(i + 1))
                       .str();
            param_assign_part += line + '\n';
        }

        // Program
        CodeGenerator code_gen;
        // Use a new scope manager because variables are scoped by bash
        std::string program_code =
            code_gen.GenCode(func_def.body, environment, scope_manager);

        auto all_code = param_assign_part + "\n\n" + program_code;
        all_code = CodeGenerator::AddIndentToEachLine(all_code);
        if (func_def.is_export) {
            code += (boost::format(kFuncDefExpoTemplate) % new_name % all_code)
                        .str();
        } else {
            code +=
                (boost::format(kFuncDefTemplate) % new_name % all_code).str();
        }
    }
    SUSHI_VISITING(ast::ReturnStmt, return_stmt) {
        // Like assignment
        ExprVisitor value_expr_visitor(
            scope_manager, environment, scope, false);
        return_stmt.value->AcceptVisitor(value_expr_visitor);

        // new_ids.merge(value_expr_visitor.new_ids);
        MergeSets(new_ids, value_expr_visitor.new_ids);

        code += value_expr_visitor.code_before;

        auto type = environment.LookUp(return_stmt.value.get());
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        switch (type_visitor.type) {
        default:
            assert(false && "Type is not supposed to be here");
        case ST::kInt:
        case ST::kUnit:
        case ST::kFd:
        case ST::kExitCode:
        case ST::kPath:
        case ST::kRelPath:
        case ST::kString:
        case ST::kChar:
        case ST::kFunc:
            code += (boost::format(kReturnStmtNotBoolTemplate) %
                     value_expr_visitor.val)
                        .str();
            break;
        case ST::kBool:
            code += (boost::format(kReturnStmtBoolTemplate) %
                     value_expr_visitor.val)
                        .str();
            break;
        case ST::kArray:
            code += (boost::format(kReturnStmtArrayTemplate) %
                     value_expr_visitor.val)
                        .str();
            break;
        case ST::kMap:
            code +=
                (boost::format(kReturnStmtMapTemplate) % value_expr_visitor.val)
                    .str();
            break;
        }
    }
    SUSHI_VISITING(ast::IfStmt, if_stmt) {
        ExprVisitor condition_visitor(scope_manager, environment, scope);
        if_stmt.condition->AcceptVisitor(condition_visitor);

        // new_ids.merge(condition_visitor.new_ids);
        MergeSets(new_ids, condition_visitor.new_ids);

        CodeGenerator true_body_gen;
        std::string true_body = true_body_gen.GenCode(
            if_stmt.true_body, environment, scope_manager);
        true_body = CodeGenerator::AddIndentToEachLine(true_body);

        auto type = environment.LookUp(if_stmt.condition.get());
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        if (if_stmt.false_body.statements.empty()) {
            code += condition_visitor.code_before;
            if (type_visitor.type == ST::kBool) {
                code += (boost::format(kIfStmtPartTemplate) %
                         condition_visitor.val % true_body)
                            .str();
            } else if (type_visitor.type == ST::kExitCode) {
                // Implicitly conversion
                code += (boost::format(kIfStmtExitCodePartTemplate) %
                         condition_visitor.val % true_body)
                            .str();
            }
        } else {
            code += condition_visitor.code_before;
            CodeGenerator false_body_gen;
            std::string false_body = false_body_gen.GenCode(
                if_stmt.false_body, environment, scope_manager);
            false_body = CodeGenerator::AddIndentToEachLine(false_body);

            code += condition_visitor.code_before;
            if (type_visitor.type == ST::kBool) {
                code += (boost::format(kIfStmtFullTemplate) %
                         condition_visitor.val % true_body % false_body)
                            .str();
            } else if (type_visitor.type == ST::kExitCode) {
                code += (boost::format(kIfStmtExitCodeFullTemplate) %
                         condition_visitor.val % true_body % false_body)
                            .str();
            }
        }
    }

    SUSHI_VISITING(ast::SwitchStmt, switch_stmt) {
        ExprVisitor switched_visitor(scope_manager, environment, scope, false);
        switch_stmt.switched->AcceptVisitor(switched_visitor);

        // new_ids.merge(switched_visitor.new_ids);
        MergeSets(new_ids, switched_visitor.new_ids);

        bool is_first_case = true;

        auto default_case = &*std::find_if(
            switch_stmt.cases.begin(), switch_stmt.cases.end(),
            [](const ast::SwitchStmt::Case &case_) -> bool {
                return case_.condition == nullptr;
            });

        std::string code_before;
        std::string temp_code;
        // each case
        for (auto &case_ : switch_stmt.cases) {
            if (default_case == &case_) continue;
            ExprVisitor case_visitor(scope_manager, environment, scope);
            case_.condition->AcceptVisitor(case_visitor);

            // new_ids.merge(case_visitor.new_ids);
            MergeSets(new_ids, case_visitor.new_ids);

            code_before += case_visitor.code_before + '\n';
            constexpr char template_[][64] = {
                "if [[ %1% -eq %2% ]]; then", "elif [[ %1% -eq %2% ]]; then",
                "if [[ `_sushi_comp_array %1% %2%` -ne 0 ]]; then",
                "elif [[ `_sushi_comp_array %1% %2%` -ne 0 ]]; then"};
            int select_template = 0;
            if (is_first_case) {
                is_first_case = false;
                select_template = 0;
            } else {
                select_template = 1;
            }
            auto type = environment.LookUp(case_.condition.get());
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);

            if (type_visitor.type == ST::kArray) {
                select_template += 2;
            }

            temp_code += (boost::format(template_[select_template]) %
                          case_visitor.val % switched_visitor.val)
                             .str();
            CodeGenerator body_gen;
            auto body_code =
                body_gen.GenCode(case_.body, environment, scope_manager);
            temp_code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
        }
        // default case
        {
            temp_code += "else";
            CodeGenerator body_gen;
            auto body_code = body_gen.GenCode(
                default_case->body, environment, scope_manager);
            temp_code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
        }
        // fi
        temp_code += "\nfi";

        code = code_before + '\n' + temp_code;
    }
    SUSHI_VISITING(ast::ForStmt, for_stmt) {
        if (for_stmt.condition.IsRange()) {
            // For to iterate
            auto scope = environment.LookUp(&program);
            auto new_name =
                scope_manager->GetNewName(for_stmt.condition.ident_name, scope);
            ExprVisitor expr_visitor(scope_manager, environment, scope, false);
            for_stmt.condition.condition->AcceptVisitor(expr_visitor);

            // new_ids.merge(expr_visitor.new_ids);
            MergeSets(new_ids, expr_visitor.new_ids);
            new_ids.insert(new_name);

            CodeGenerator code_gen;
            auto for_body =
                code_gen.GenCode(for_stmt.body, environment, scope_manager);
            for_body = CodeGenerator::AddIndentToEachLine(for_body);
            auto code_before = expr_visitor.code_before;
            code += code_before;
            code += '\n' + (boost::format(kForStmtIterTemplate) % new_name %
                            expr_visitor.val % for_body)
                               .str();
        } else {
            // For as while
            ExprVisitor expr_visitor(scope_manager, environment, scope);
            for_stmt.condition.condition->AcceptVisitor(expr_visitor);

            // new_ids.merge(expr_visitor.new_ids);
            MergeSets(new_ids, expr_visitor.new_ids);

            auto type = environment.LookUp(for_stmt.condition.condition.get());
            TypeVisitor type_visitor;
            type->AcceptVisitor(type_visitor);

            CodeGenerator code_gen;
            auto for_body =
                code_gen.GenCode(for_stmt.body, environment, scope_manager);
            for_body = CodeGenerator::AddIndentToEachLine(for_body);
            auto code_before = expr_visitor.code_before;
            code += code_before;
            if (type_visitor.type == ST::kBool) {
                code += '\n' + (boost::format(kForStmtWhileTemplate) %
                                expr_visitor.val % for_body)
                                   .str();
            } else if (type_visitor.type == ST::kExitCode) {
                // Implicitly conversion
                code += '\n' + (boost::format(kForStmtWhileExitCodeTemplate) %
                                expr_visitor.val % for_body)
                                   .str();
            }
        }
    }
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt) {
        if (loop_control_stmt.control_type ==
            ast::LoopControlStmt::Value::kBreak) {
            code = "break " + std::to_string(loop_control_stmt.level);
        } else if (
            loop_control_stmt.control_type ==
            ast::LoopControlStmt::Value::kContinue) {
            code = "continue " + std::to_string(loop_control_stmt.level);
        }
    }
};

} // namespace code_generation
} // namespace sushi

#endif
