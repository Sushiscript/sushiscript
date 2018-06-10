#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include <string>
#include <map>
#include "sushi/ast.h"
#include "sushi/scope.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"

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
    // bool is_lit;

    CodeGenExprVisitor(bool is_left_value = false) : is_left_value(is_left_value) {}

    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unary_expr);
    SUSHI_VISITING(ast::BinaryExpr, binary_expr);
    SUSHI_VISITING(ast::CommandLike, cmd_like);
    SUSHI_VISITING(ast::Indexing, indexing);
  private:
    bool is_left_value;
};

class ScopeManager {
    std::map<const std::string &, int> origin_name_to_int;
    std::map<std::pair<const Scope *, const std::string &>, std::string> new_names_map;
  public:
    ScopeManager() {
        origin_name_to_int["_sushi_t_"] = -1;
    }

    std::string GetNewTemp(const Scope * scope) {
        std::string str = "_sushi_t_";
        int new_int = origin_name_to_int[str];
        ++origin_name_to_int[str];
        if (new_int == -1) {
            return str;
        } else {
            auto new_name = str + std::to_string(new_int) + '_';
            return new_name;
        }
    }

    std::string GetNewName(const std::string & identifier, const Scope * scope) {
        if (origin_name_to_int.find(identifier) != origin_name_to_int.end()
        || origin_name_to_int[identifier] == -1) {
            new_names_map[std::make_pair(scope, identifier)] = identifier;
            return identifier;
        } else {
            int new_int = origin_name_to_int[identifier];
            auto new_name = identifier + "_scope_" + std::to_string(new_int);
            new_names_map[std::make_pair(scope, identifier)] = new_name;
            ++origin_name_to_int[identifier];
            return new_name;
        }
    }

    void UnsetTemp(const std::string & new_name) {
        --origin_name_to_int[new_name];
    }

    void UnsetName(const std::string & new_name) {
        --origin_name_to_int[new_name];
        origin_name_to_int.erase(new_name);
    }

    std::string FindNewName(const std::string & identifier, const Scope * scope) {
        auto find_res = new_names_map.find(std::make_pair(scope, identifier));
        if (find_res != new_names_map.end()) {
            return find_res->second;
        } else {
            throw "Cannot find the identifier";
        }
    }
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
constexpr char kIfStmtPartTemplate[] = "if (%1%); then\n%2%\nfi";
constexpr char kIfStmtFullTemplate[] = "if (%1%); then\n%2%\nelse\n%3%\nfi";

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
        CodeGenExprVisitor lvalue_expr_visitor(true);
        assignment.lvalue->AcceptVisitor(lvalue_expr_visitor);
        CodeGenExprVisitor rvalue_expr_visitor(false);
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
        CodeGenExprVisitor expr_visitor;
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
        std::string program_code = code_gen.GenCode(program, environment, scope_manager);
        program_code = CodeGenerator::AddIndentToEachLine(program_code);

        auto all_code = param_assign_part + "\n\n" + program_code;
        code += (boost::format(kFuncDefTemplate) % new_name % all_code).str();
    }
    SUSHI_VISITING(ast::ReturnStmt, return_stmt) {
        // Like assignment
        CodeGenExprVisitor value_expr_visitor(false);
        return_stmt.value->AcceptVisitor(value_expr_visitor);
        code += value_expr_visitor.code_before;
        // TODO: Judge whether return value is Bool type
        code += (boost::format(kReturnStmtBoolTemplate) % value_expr_visitor.val).str();
    }
    SUSHI_VISITING(ast::IfStmt, if_stmt);
    SUSHI_VISITING(ast::SwitchStmt, switch_stmt);
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
