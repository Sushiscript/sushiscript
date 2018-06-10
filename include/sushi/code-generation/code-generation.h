#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include <string>
#include <map>
#include <algorithm>
#include "sushi/ast.h"
#include "sushi/scope.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "./scope-manager.h"

namespace sushi {

struct CodeGenTypeExprVisitor : public ast::TypeExprVisitor::Const {
    std::string type_expr_str;

    SUSHI_VISITING(ast::TypeLit, type_lit);
    SUSHI_VISITING(ast::ArrayType, array_type);
    SUSHI_VISITING(ast::MapType, map_type);
    SUSHI_VISITING(ast::FunctionType, function_type);
};

struct CodeGenExprVisitor : public ast::ExpressionVisitor::Const {
    std::string val;
    std::string code_before;
    std::string expr_type;

    CodeGenExprVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        bool is_left_value = false
        ) : is_left_value(is_left_value), scope_manager(scope_manager) {}

    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unary_expr);
    SUSHI_VISITING(ast::BinaryExpr, binary_expr);
    SUSHI_VISITING(ast::CommandLike, cmd_like);
    SUSHI_VISITING(ast::Indexing, indexing);
  private:
    bool is_left_value;
    std::shared_ptr<ScopeManager> scope_manager;
};

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program & program,
        const Environment & environment,
        std::shared_ptr<ScopeManager> scope_manager = nullptr);
    static std::string AddIndentToEachLine(const std::string & str) {
        std::vector<std::string> res;
        boost::algorithm::split(res, str, boost::algorithm::is_any_of("\n"));
        std::string ret;
        for (auto & s : res) {
            ret += kIndentString + s + '\n';
        }
        return ret;
    }
};

constexpr char kIndentString[] = "    ";

constexpr char kAssignTemplate[] = "%1%=%2%";
constexpr char kVarDefFullTemplate[] = "local -%1% %2%=%3%";
constexpr char kVarDefPartTemplate[] = "local %1%=%2%";
constexpr char kVarDefExpoTemplate[] = "declare -x%1% %2%=%3%";
constexpr char kFuncDefTemplate[] = "%1% () {\n%2%\n}";
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

struct CodeGenStmtVisitor : public ast::StatementVisitor::Const {
    std::string code;
    const Environment & environment;
    const ast::Program & program;
    std::shared_ptr<ScopeManager> scope_manager;

    std::vector<std::string> identifiers_to_unset;

    CodeGenStmtVisitor(
        const Environment & environment, const ast::Program & program, std::shared_ptr<ScopeManager> scope_manager)
        : environment(environment), program(program), scope_manager(scope_manager) {}

    SUSHI_VISITING(ast::Assignment, assignment) {
        CodeGenExprVisitor lvalue_expr_visitor(scope_manager, true);
        assignment.lvalue->AcceptVisitor(lvalue_expr_visitor);
        CodeGenExprVisitor rvalue_expr_visitor(scope_manager, false);
        assignment.value->AcceptVisitor(rvalue_expr_visitor);
        code += lvalue_expr_visitor.code_before + '\n' + rvalue_expr_visitor.code_before + '\n';
        code += (boost::format(kAssignTemplate) % lvalue_expr_visitor.val
                                                % rvalue_expr_visitor.val).str();
    }
    SUSHI_VISITING(ast::Expression, expression);
    SUSHI_VISITING(ast::VariableDef, var_def) {
        const Scope * scope = environment.LookUp(&program);
        auto new_name = scope_manager->GetNewName(var_def.name, scope);
        identifiers_to_unset.push_back(new_name);
        CodeGenTypeExprVisitor type_visitor;
        var_def.type->AcceptVisitor(type_visitor);
        CodeGenExprVisitor expr_visitor(scope_manager);
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
        const Scope * scope = environment.LookUp(&program);
        CodeGenTypeExprVisitor ret_type_visitor;
        func_def.ret_type->AcceptVisitor(ret_type_visitor);
        auto new_name = scope_manager->GetNewName(func_def.name, scope);

        // Params assignment
        std::string param_assign_part;
        for (int i = 0; i < func_def.params.size(); ++i) {
            auto & param = func_def.params[i];
            std::string line;
            CodeGenTypeExprVisitor type_visitor;
            param.type->AcceptVisitor(type_visitor);
            if (type_visitor.type_expr_str != "") {
                std::string param_right;
                if (type_visitor.type_expr_str == "a") {
                    param_right = "($" + std::to_string(i + 1) + ")";
                } else if (type_visitor.type_expr_str == "A") {
                    constexpr char template_[] = R"foo((); eval "%1%=($%2%)")foo";
                    param_right = (boost::format(template_) % param.name
                                                            % (i + 1)).str();
                } else {
                    param_right = "$" + std::to_string(i + 1);
                }
                line += (boost::format(kVarDefFullTemplate) % type_visitor.type_expr_str
                                                            % param.name
                                                            % param_right).str();
            }
            param_assign_part += line + '\n';
        }

        // Program
        CodeGenerator code_gen;
        std::string program_code = code_gen.GenCode(func_def.body, environment, scope_manager);
        program_code = CodeGenerator::AddIndentToEachLine(program_code);

        auto all_code = param_assign_part + "\n\n" + program_code;
        code += (boost::format(kFuncDefTemplate) % new_name % all_code).str();
    }
    SUSHI_VISITING(ast::ReturnStmt, return_stmt) {
        // Like assignment
        CodeGenExprVisitor value_expr_visitor(scope_manager, false);
        return_stmt.value->AcceptVisitor(value_expr_visitor);
        code += value_expr_visitor.code_before;
        // TODO: Judge whether return value is Bool type
        code += (boost::format(kReturnStmtBoolTemplate) % value_expr_visitor.val).str();
    }
    SUSHI_VISITING(ast::IfStmt, if_stmt) {
        CodeGenExprVisitor condition_visitor(scope_manager);
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
    struct SwitchCaseExprVisitor : public ast::ExpressionVisitor::Const {
        /*
         * Translate Bool to `[[ condition ]]`
         * Function Bool to `func_name <t_Bool>`
         */
        std::string val;
        std::string code_before;

        SwitchCaseExprVisitor(
            std::shared_ptr<ScopeManager> scope_manager
            ) : scope_manager(scope_manager) {}

        SUSHI_VISITING(ast::Variable, variable);
        SUSHI_VISITING(ast::Literal, literal);
        SUSHI_VISITING(ast::UnaryExpr, unary_expr);
        SUSHI_VISITING(ast::BinaryExpr, binary_expr);
        SUSHI_VISITING(ast::CommandLike, cmd_like);
        SUSHI_VISITING(ast::Indexing, indexing);
      private:
        std::shared_ptr<ScopeManager> scope_manager;
    };
    SUSHI_VISITING(ast::SwitchStmt, switch_stmt) {
        CodeGenExprVisitor switched_visitor(scope_manager, false);
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
            SwitchCaseExprVisitor case_visitor(scope_manager);
            case_.condition->AcceptVisitor(case_visitor);
            code_before += case_visitor.code_before;
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
    SUSHI_VISITING(ast::ForStmt, for_stmt);
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt) {
        if (loop_control_stmt.control_type == ast::LoopControlStmt::Value::kBreak) {
            code = "break " + std::to_string(loop_control_stmt.level);
        } else if (loop_control_stmt.control_type == ast::LoopControlStmt::Value::kContinue) {
            code = "continue " + std::to_string(loop_control_stmt.level);
        }
    }
};

}  // namespace sushi

#endif
