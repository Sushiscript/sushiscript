#ifndef SUSHI_CODE_GEN_STMT_VISITOR_H_
#define SUSHI_CODE_GEN_STMT_VISITOR_H_

#include "./expr-visitor.h"
#include "./type-visitor.h"
#include "./code-generation.h"

namespace sushi {
namespace code_generation {

constexpr char kAssignTemplate[] = "%1%=%2%";
constexpr char kAssignArrayTemplate[] = "%1%=(%2%)";
constexpr char kAssignMapTemplate[] = "eval \"%1%=(%2%)\"";
constexpr char kVarDefFullTemplate[] = "local -%1% %2%=%3%";
constexpr char kVarDefPartTemplate[] = "local %1%=%2%";
constexpr char kVarDefExpoTemplate[] = "declare -x%1% %2%=%3%";
constexpr char kFuncDefTemplate[] = "%1% () {\n%2%\n}";
constexpr char kFuncDefExpoTemplate[] = "%1% () {\n%2%\n}\nexport %1%";
constexpr char kReturnStmtNotBoolTemplate[] = "_sushi_func_ret_=%1%; return 0";
constexpr char kReturnStmtBoolTemplate[] =
R"(_sushi_func_ret_=%1%
if [[ _sushi_func_ret_ -ne 0 ]]; then
    return 0
else
    return 1
fi)";
constexpr char kIfStmtPartTemplate[] = "if [[ %1% ]]; then\n%2%\nfi";
constexpr char kIfStmtFullTemplate[] = "if [[ %1% ]]; then\n%2%\nelse\n%3%\nfi";
constexpr char kForStmtIterTemplate[] = "for %1% in %2%; do\n%3%\ndone";
constexpr char kForStmtWhileTemplate[] = "while %1%; do\n%2%\ndone";

struct StmtVisitor : public ast::StatementVisitor::Const {
    std::string code;
    const scope::Environment & environment;
    const ast::Program & program;
    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Scope * scope;

    std::vector<std::string> identifiers_to_unset;

    StmtVisitor(
        const scope::Environment & environment, const ast::Program & program, std::shared_ptr<ScopeManager> scope_manager)
        : environment(environment), program(program), scope_manager(scope_manager) {
            scope = environment.LookUp(&program);
        }

    SUSHI_VISITING(ast::Assignment, assignment) {
        ExprVisitor lvalue_expr_visitor(scope_manager, environment, scope, true);
        assignment.lvalue->AcceptVisitor(lvalue_expr_visitor);
        ExprVisitor rvalue_expr_visitor(scope_manager, environment, scope, false);
        assignment.value->AcceptVisitor(rvalue_expr_visitor);

        auto type = environment.LookUp(assignment.value.get());

        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);

        using ST = TypeVisitor::SimplifiedType;
        code += lvalue_expr_visitor.code_before + '\n' + rvalue_expr_visitor.code_before + '\n';
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
            code += (boost::format(kAssignTemplate) % lvalue_expr_visitor.val
                                                    % rvalue_expr_visitor.val).str();
            break;
        case ST::kMap:
            code += (boost::format(kAssignMapTemplate) % lvalue_expr_visitor.val
                                                       % rvalue_expr_visitor.val).str();
            break;
        case ST::kArray:
            code += (boost::format(kAssignArrayTemplate) % lvalue_expr_visitor.val
                                                         % rvalue_expr_visitor.val).str();
            break;
        }

    }
    SUSHI_VISITING(ast::Expression, expression) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        expression.AcceptVisitor(expr_visitor);
        code += expr_visitor.code_before;
        code += '\n' + expr_visitor.val;
    }
    SUSHI_VISITING(ast::VariableDef, var_def) {
        const scope::Scope * scope = environment.LookUp(&program);
        auto new_name = scope_manager->GetNewName(var_def.name, scope);
        identifiers_to_unset.push_back(new_name);
        TypeExprVisitor type_visitor;
        var_def.type->AcceptVisitor(type_visitor);
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        var_def.value->AcceptVisitor(expr_visitor);
        code += expr_visitor.code_before;
        if (type_visitor.type_expr_str != "") {
            if (var_def.is_export) {
                code += (boost::format(kVarDefExpoTemplate) % type_visitor.type_expr_str
                                                            % new_name
                                                            % expr_visitor.val).str();
            } else {
                code += (boost::format(kVarDefFullTemplate) % type_visitor.type_expr_str
                                                            % new_name
                                                            % expr_visitor.val).str();
            }
        } else {
            if (var_def.is_export) {
                code += (boost::format(kVarDefExpoTemplate) % ""
                                                            % new_name
                                                            % expr_visitor.val).str();
            } else {
                code += (boost::format(kVarDefPartTemplate) % new_name
                                                            % expr_visitor.val).str();
            }
        }
    }
    SUSHI_VISITING(ast::FunctionDef, func_def) {
        const scope::Scope * scope = environment.LookUp(&program);
        TypeExprVisitor ret_type_visitor;
        func_def.ret_type->AcceptVisitor(ret_type_visitor);
        auto new_name = scope_manager->GetNewName(func_def.name, scope);

        // Params assignment
        std::string param_assign_part;
        constexpr char kParamAssignTemplate[] = "local -n %1%=%2%";
        for (int i = 0; i < func_def.params.size(); ++i) {
            auto & param = func_def.params[i];
            std::string line;
            line = (boost::format(kParamAssignTemplate) % param.name % std::to_string(i + 1)).str();
            param_assign_part += line + '\n';
        }

        // Program
        CodeGenerator code_gen;
        std::string program_code = code_gen.GenCode(func_def.body, environment, scope_manager);
        program_code = CodeGenerator::AddIndentToEachLine(program_code);

        auto all_code = param_assign_part + "\n\n" + program_code;
        if (func_def.is_export) {
            code += (boost::format(kFuncDefExpoTemplate) % new_name % all_code).str();
        } else {
            code += (boost::format(kFuncDefTemplate) % new_name % all_code).str();
        }
    }
    SUSHI_VISITING(ast::ReturnStmt, return_stmt) {
        // Like assignment
        ExprVisitor value_expr_visitor(scope_manager, environment, scope, false);
        return_stmt.value->AcceptVisitor(value_expr_visitor);
        code += value_expr_visitor.code_before;
        // TODO: Judge whether return value is Bool type
        code += (boost::format(kReturnStmtBoolTemplate) % value_expr_visitor.val).str();
    }
    SUSHI_VISITING(ast::IfStmt, if_stmt) {
        ConditionExprVisitor condition_visitor(scope_manager, environment, scope);
        if_stmt.condition->AcceptVisitor(condition_visitor);

        CodeGenerator true_body_gen;
        std::string true_body = true_body_gen.GenCode(if_stmt.true_body, environment, scope_manager);
        true_body = CodeGenerator::AddIndentToEachLine(true_body);

        if (if_stmt.false_body.statements.empty()) {
            code += condition_visitor.code_before;
            code += (boost::format(kIfStmtPartTemplate) % condition_visitor.val
                                                        % true_body).str();
        } else {
            code += condition_visitor.code_before;
            CodeGenerator false_body_gen;
            std::string false_body = false_body_gen.GenCode(if_stmt.false_body, environment, scope_manager);
            false_body = CodeGenerator::AddIndentToEachLine(false_body);

            code += condition_visitor.code_before;
            code += (boost::format(kIfStmtFullTemplate) % condition_visitor.val
                                                        % true_body
                                                        % false_body).str();
        }
    }

    SUSHI_VISITING(ast::SwitchStmt, switch_stmt) {
        ExprVisitor switched_visitor(scope_manager, environment, scope, false);
        switch_stmt.switched->AcceptVisitor(switched_visitor);

        bool is_first_case = true;

        auto default_case =  &*std::find(
            switch_stmt.cases.begin(),
            switch_stmt.cases.end(),
            [](const ast::SwitchStmt::Case & case_) -> bool {
                return case_.condition == nullptr;
            }
        );

        std::string code_before;
        // each case
        for (auto & case_ : switch_stmt.cases) {
            if (default_case == &case_) continue;
            SwitchCaseExprVisitor case_visitor(scope_manager, environment, scope, switched_visitor.val);
            case_.condition->AcceptVisitor(case_visitor);
            code_before += case_visitor.code_before + '\n';
            if (is_first_case) {
                is_first_case = false;
                constexpr char template_[] = "if %1%; then";
                code += (boost::format(template_) % case_visitor.val).str();
                CodeGenerator body_gen;
                auto body_code = body_gen.GenCode(case_.body, environment, scope_manager);
                code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
            } else {
                constexpr char template_[] = "elif %1%";
                code += (boost::format(template_) % case_visitor.val).str();
                CodeGenerator body_gen;
                auto body_code = body_gen.GenCode(case_.body, environment, scope_manager);
                code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
            }
        }
        // default case
        {
            code += "else";
            CodeGenerator body_gen;
            auto body_code = body_gen.GenCode(default_case->body, environment, scope_manager);
            code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
        }
        // fi
        code += "\nfi";
    }
    SUSHI_VISITING(ast::ForStmt, for_stmt) {
        if (for_stmt.condition.IsRange()) {
            // For to iterate
            auto scope = environment.LookUp(&program);
            auto new_name = scope_manager->GetNewName(for_stmt.condition.ident_name, scope);
            ExprVisitor expr_visitor(scope_manager, environment, scope, false);
            for_stmt.condition.condition->AcceptVisitor(expr_visitor);
            CodeGenerator code_gen;
            auto for_body = code_gen.GenCode(for_stmt.body, environment, scope_manager);
            for_body = CodeGenerator::AddIndentToEachLine(for_body);
            auto code_before = expr_visitor.code_before;
            code += code_before;
            code += '\n' + (boost::format(kForStmtIterTemplate) % new_name
                                                                % expr_visitor.val
                                                                % for_body).str();
        } else {
            // For as while
            ConditionExprVisitor expr_visitor(scope_manager, environment, scope);
            for_stmt.condition.condition->AcceptVisitor(expr_visitor);
            CodeGenerator code_gen;
            auto for_body = code_gen.GenCode(for_stmt.body, environment, scope_manager);
            for_body = CodeGenerator::AddIndentToEachLine(for_body);
            auto code_before = expr_visitor.code_before;
            code += code_before;
            code += '\n' + (boost::format(kForStmtWhileTemplate) % expr_visitor.val
                                                                 % for_body).str();
        }
    }
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt) {
        if (loop_control_stmt.control_type == ast::LoopControlStmt::Value::kBreak) {
            code = "break " + std::to_string(loop_control_stmt.level);
        } else if (loop_control_stmt.control_type == ast::LoopControlStmt::Value::kContinue) {
            code = "continue " + std::to_string(loop_control_stmt.level);
        }
    }
};

} // namespace code_generation
} // namespace sushi


#endif
