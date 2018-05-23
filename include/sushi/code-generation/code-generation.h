#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include <string>
#include "sushi/ast.h"
#include "sushi/scope.h"
#include "boost/format.hpp"

namespace sushi {

struct CodeGenTypeExprVisitor : public ast::TypeExprVisitor::Const {
    std::string type_expr_str;

    SUSHI_VISITING(ast::TypeLit, type_lit);
    SUSHI_VISITING(ast::ArrayType, array_type);
    SUSHI_VISITING(ast::MapType, map_type);
};

struct CodeGenExprVisitor : public ast::ExpressionVisitor::Const {
    std::string expr_str;

    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unary_expr);
    SUSHI_VISITING(ast::BinaryExpr, binary_expr);
    SUSHI_VISITING(ast::CommandLike, cmd_like);
    SUSHI_VISITING(ast::Indexing, indexing);
};

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program & program,
        const Environment & environment);
};

constexpr char kVarDedTemplate[] = "\nlocal -%1% %2%=%3%";
constexpr char kFuncDefTemplate[] = "\nfunction () {\n%1%}";
constexpr char kIfStmtTemplate[] = "\nif (%1%); then\n%2%\nfi";
constexpr char kIfStmtFullTemplate[] = "\nif (%1%); then\n%2%else\n%3%\nfi";
constexpr char kReturnStmtTemplate[] = "\nreturn %1%";

struct CodeGenStmtVisitor : public ast::StatementVisitor::Const {
    std::string code;
    const Environment & environment;
    const ast::Program & program;

    CodeGenStmtVisitor(
        const Environment & environment, const ast::Program & program)
        : environment(environment), program(program) {}

    SUSHI_VISITING(ast::VariableDef, var_def) {
        // TODO: Check if there is another variable with the same name

        CodeGenTypeExprVisitor type_visitor;
        var_def.type->AcceptVisitor(type_visitor);
        CodeGenExprVisitor value_visitor;
        var_def.value->AcceptVisitor(value_visitor);

        code = (boost::format(kVarDedTemplate)
                % (type_visitor.type_expr_str + (var_def.is_export ? "x" : ""))
                % var_def.name
                % value_visitor.expr_str).str();
    }
    SUSHI_VISITING(ast::FunctionDef, func_def);
    SUSHI_VISITING(ast::IfStmt, if_stmt) {
        CodeGenerator code_gen;
        CodeGenExprVisitor expr_visitor;
        if_stmt.condition->AcceptVisitor(expr_visitor);
        if (if_stmt.false_body == nullptr) {
            auto true_str = code_gen.GenCode(*if_stmt.true_body, environment);
            code = (boost::format(kIfStmtTemplate)
                    % expr_visitor.expr_str
                    % true_str).str();
        } else {
            auto true_str = code_gen.GenCode(*if_stmt.true_body, environment);
            auto false_str = code_gen.GenCode(*if_stmt.false_body, environment);
            code = (boost::format(kIfStmtFullTemplate)
                    % expr_visitor.expr_str
                    % true_str
                    % false_str).str();
        }
    }
    SUSHI_VISITING(ast::ReturnStmt, return_stmt);
    SUSHI_VISITING(ast::SwitchStmt, switch_stmt);
    SUSHI_VISITING(ast::ForStmt, for_stmt);
    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt);
    SUSHI_VISITING(ast::Expression, expression);
};

}  // namespace sushi

#endif
