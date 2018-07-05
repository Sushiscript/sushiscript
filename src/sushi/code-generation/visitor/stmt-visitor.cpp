#include "sushi/code-generation/visitor/stmt-visitor.h"
#include "sushi/code-generation/visitor/expr-visitor.h"
#include "sushi/code-generation/util.h"
#include "sushi/code-generation/code-generation.h"

#define STMT_VISITING_IMPL(T, t) void StmtVisitor::Visit(const T &t)

namespace sushi {
namespace code_generation {

STMT_VISITING_IMPL(ast::Assignment, assignment) {
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

    auto lvalue_type = environment.LookUp(assignment.lvalue.get());
    auto rvalue_type = environment.LookUp(assignment.value.get());

    TypeVisitor type_visitor;
    rvalue_type->AcceptVisitor(type_visitor);
    auto rval_simplified_type = type_visitor.type;
    lvalue_type->AcceptVisitor(type_visitor);
    auto lval_simplified_type = type_visitor.type;

    code += lvalue_expr_visitor.code_before + '\n' +
            rvalue_expr_visitor.code_before + '\n';

    std::string rval_str = rvalue_expr_visitor.val;
    switch (rval_simplified_type) {
    default: assert(false && "Type is not supposed to be here");
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
        if (lval_simplified_type == ST::kExitCode &&
            rval_simplified_type == ST::kBool) {
            rval_str = ExitCodeExprToBool(rval_str);
        } else if (
            lval_simplified_type == ST::kExitCode &&
            rval_simplified_type == ST::kInt) {
            rval_str = ExitCodeExprToInt(rval_str);
        } else if (
            lval_simplified_type == ST::kRelPath &&
            rval_simplified_type == ST::kPath) {
            rval_str = RelPathExprToPath(rval_str);
        }
        code += (boost::format(kAssignTemplate) %
                    lvalue_expr_visitor.raw_id % rval_str)
                    .str();
        break;
    case ST::kMap:
        code += (boost::format(kAssignMapTemplate) %
                    lvalue_expr_visitor.raw_id % rval_str)
                    .str();
        break;
    case ST::kArray:
        code += (boost::format(kAssignArrayTemplate) %
                    lvalue_expr_visitor.raw_id % rval_str)
                    .str();
        break;
    }
}
STMT_VISITING_IMPL(ast::Expression, expression) {
    ExprVisitor expr_visitor(scope_manager, environment, scope);
    expression.AcceptVisitor(expr_visitor);

    // new_ids.merge(expr_visitor.new_ids);
    MergeSets(new_ids, expr_visitor.new_ids);

    code += expr_visitor.code_before;
    // expression here does nothing, cmd-like is invoked in code_bofore
    // code += '\n' + expr_visitor.val;
}
STMT_VISITING_IMPL(ast::VariableDef, var_def) {
    const scope::Scope *scope = environment.LookUp(&program);
    auto new_name = scope_manager->GetNewName(var_def.name, scope);

    ExprVisitor expr_visitor(scope_manager, environment, scope);
    var_def.value->AcceptVisitor(expr_visitor);

    std::unique_ptr<type::Type> var_type;
    ST var_simplified_type;
    if (var_def.type) {
        var_type = var_def.type->ToType();
        TypeVisitor visitor;
        var_type->AcceptVisitor(visitor);
        var_simplified_type = visitor.type;
    }

    auto type = environment.LookUp(var_def.value.get());
    TypeVisitor type_visitor;
    type->AcceptVisitor(type_visitor);

    auto rval_simplified_type = type_visitor.type;

    // new_ids.merge(expr_visitor.new_ids);
    MergeSets(new_ids, expr_visitor.new_ids);
    if (!var_def.is_export) new_ids.insert(new_name);

    code += expr_visitor.code_before + '\n';

    std::string rval_str;

    switch (rval_simplified_type) {
    default: assert(false && "Type is not supposed to be here");
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
        if (var_def.type && var_simplified_type == ST::kExitCode &&
            rval_simplified_type == ST::kBool) {
            rval_str = ExitCodeExprToBool(rval_str);
        } else if (
            var_def.type && var_simplified_type == ST::kExitCode &&
            rval_simplified_type == ST::kInt) {
            rval_str = ExitCodeExprToInt(rval_str);
        } else if (
            var_def.type && var_simplified_type == ST::kRelPath &&
            rval_simplified_type == ST::kPath) {
            rval_str = RelPathExprToPath(rval_str);
        }
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
STMT_VISITING_IMPL(ast::FunctionDef, func_def) {
    const scope::Scope *scope = environment.LookUp(&program);
    auto new_name = scope_manager->GetNewName(func_def.name, scope);
    auto body_scope = environment.LookUp(&func_def.body);

    if (!func_def.is_export) new_ids.insert(new_name);

    // Params assignment
    std::string param_assign_part;
    constexpr char kParamAssignTemplate[] = "local -n %1%=%2%";
    constexpr char kSimpleParamAssignTemplate[] = "local %1%=%2%";
    for (int i = 0; i < func_def.params.size(); ++i) {
        auto &param = func_def.params[i];
        auto new_param_name = scope_manager->GetNewName(param.name, body_scope);
        std::string line;

        auto type = GetTypeFromTypeExpr(param.type.get());

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
            line = (boost::format(kSimpleParamAssignTemplate) % new_param_name %
                    ('$' + std::to_string(i + 1)))
                        .str();
            break;
        case ST::kArray:
        case ST::kMap:
            line = (boost::format(kParamAssignTemplate) % new_param_name %
                    ('$' + std::to_string(i + 1)))
                        .str();
            break;
        }

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
STMT_VISITING_IMPL(ast::ReturnStmt, return_stmt) {
    // TODO: Implicitly Conversion not finished
    // Like assignment
    ExprVisitor value_expr_visitor(
        scope_manager, environment, scope, false);
    return_stmt.value->AcceptVisitor(value_expr_visitor);

    // new_ids.merge(value_expr_visitor.new_ids);
    MergeSets(new_ids, value_expr_visitor.new_ids);

    code += value_expr_visitor.code_before + '\n';

    auto type = environment.LookUp(return_stmt.value.get());
    TypeVisitor type_visitor;
    type->AcceptVisitor(type_visitor);

    switch (type_visitor.type) {
    default: assert(false && "Type is not supposed to be here");
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
STMT_VISITING_IMPL(ast::IfStmt, if_stmt) {
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
        code += condition_visitor.code_before + '\n';
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
        code += condition_visitor.code_before + '\n';
        CodeGenerator false_body_gen;
        std::string false_body = false_body_gen.GenCode(
            if_stmt.false_body, environment, scope_manager);
        false_body = CodeGenerator::AddIndentToEachLine(false_body);

        code += condition_visitor.code_before + '\n';
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

STMT_VISITING_IMPL(ast::SwitchStmt, switch_stmt) {
    ExprVisitor switched_visitor(scope_manager, environment, scope, false);
    switch_stmt.switched->AcceptVisitor(switched_visitor);

    // new_ids.merge(switched_visitor.new_ids);
    MergeSets(new_ids, switched_visitor.new_ids);

    bool is_first_case = true;

    const ast::SwitchStmt::Case *default_case = nullptr;
    auto default_case_iter = std::find_if(
        switch_stmt.cases.begin(), switch_stmt.cases.end(),
        [](const ast::SwitchStmt::Case &case_) -> bool {
            return case_.condition == nullptr;
        });
    if (default_case_iter != switch_stmt.cases.end()) {
        default_case = &*default_case_iter;
    }

    auto switched_type = environment.LookUp(switch_stmt.switched.get());
    TypeVisitor visitor;
    switched_type->AcceptVisitor(visitor);
    auto switched_simplified_type = visitor.type;

    std::string code_before;
    std::string temp_code;
    // each case
    for (auto &case_ : switch_stmt.cases) {
        if (default_case == &case_) continue;
        ExprVisitor case_visitor(scope_manager, environment, scope);
        case_.condition->AcceptVisitor(case_visitor);

        // new_ids.merge(case_visitor.new_ids);
        MergeSets(new_ids, case_visitor.new_ids);

        auto case_type = environment.LookUp(case_.condition.get());
        TypeVisitor visitor;
        case_type->AcceptVisitor(visitor);
        auto case_simplified_type = visitor.type;

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

        std::string case_str = case_visitor.val;

        if (case_simplified_type == ST::kExitCode &&
            switched_simplified_type == ST::kBool) {
            case_str = ExitCodeExprToBool(case_str);
        } else if (
            case_simplified_type == ST::kExitCode &&
            switched_simplified_type == ST::kInt) {
            case_str = ExitCodeExprToInt(case_str);
        } else if (
            case_simplified_type == ST::kRelPath &&
            switched_simplified_type == ST::kPath) {
            case_str = RelPathExprToPath(case_str);
        }

        temp_code += (boost::format(template_[select_template]) % case_str %
                        switched_visitor.val)
                            .str();
        CodeGenerator body_gen;
        auto body_code =
            body_gen.GenCode(case_.body, environment, scope_manager);
        temp_code += '\n' + CodeGenerator::AddIndentToEachLine(body_code);
    }
    // default case
    {
        if (default_case) {
            temp_code += "else";
            CodeGenerator body_gen;
            auto body_code = body_gen.GenCode(
                default_case->body, environment, scope_manager);
            temp_code +=
                '\n' + CodeGenerator::AddIndentToEachLine(body_code);
        }
    }
    // fi
    temp_code += "\nfi";

    code = code_before + '\n' + temp_code;
}
STMT_VISITING_IMPL(ast::ForStmt, for_stmt) {
    if (for_stmt.condition.IsRange()) {
        // For to iterate
        auto scope = environment.LookUp(&program);
        auto body_scope = environment.LookUp(&for_stmt.body);
        auto new_name = scope_manager->GetNewName(
            for_stmt.condition.ident_name, body_scope);
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
STMT_VISITING_IMPL(ast::LoopControlStmt, loop_control_stmt) {
    if (loop_control_stmt.control_type ==
        ast::LoopControlStmt::Value::kBreak) {
        code = "break " + std::to_string(loop_control_stmt.level);
    } else if (
        loop_control_stmt.control_type ==
        ast::LoopControlStmt::Value::kContinue) {
        code = "continue " + std::to_string(loop_control_stmt.level);
    }
}

}
}
