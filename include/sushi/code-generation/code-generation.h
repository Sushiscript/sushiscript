#include <string>
#include "sushi/ast.h"
#include "sushi/scope.h"
#include "boost/format.hpp"

namespace sushi {

struct CodeGenTypeExprVisitor : public ast::TypeExprVisitor::Const {
    std::string typeExprStr;

    SUSHI_VISITING(ast::TypeLit, typeLit);
    SUSHI_VISITING(ast::ArrayType, arrayType);
    SUSHI_VISITING(ast::MapType, mapType);
};

struct CodeGenExprVisitor : public ast::ExpressionVisitor::Const {
    std::string exprStr;

    SUSHI_VISITING(ast::Variable, variable);
    SUSHI_VISITING(ast::Literal, literal);
    SUSHI_VISITING(ast::UnaryExpr, unaryExpr);
    SUSHI_VISITING(ast::BinaryExpr, binaryExpr);
    SUSHI_VISITING(ast::CommandLike, cmdLike);
    SUSHI_VISITING(ast::Indexing, indexing);
};

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program & program,
        const Environment & environment);
};

constexpr char VAR_DEF_TEMPLATE[] = "local -%1% %2%=%3%";
constexpr char FUNC_DEF_TEMPLATE[] = "function (%1%) {\n%2%}";

struct CodeGenStmtVisitor : public ast::StatementVisitor::Const {
    std::string code;
    const Environment & environment;
    const ast::Program & program;

    CodeGenStmtVisitor(
        const Environment & environment, const ast::Program & program)
        : environment(environment), program(program) {}

    SUSHI_VISITING(ast::VariableDef, varDef) {
        // TODO: Check if there is another variable with the same name

        CodeGenTypeExprVisitor typeVisitor;
        varDef.type->AcceptVisitor(typeVisitor);
        CodeGenExprVisitor valueVisitor;
        varDef.value->AcceptVisitor(valueVisitor);

        code = (boost::format(VAR_DEF_TEMPLATE)
                % (typeVisitor.typeExprStr + (varDef.is_export ? "x" : ""))
                % varDef.name
                % valueVisitor.exprStr).str();
    }
    SUSHI_VISITING(ast::FunctionDef, funcDef) {
        // TODO: Check if there is another function with the same name

        std::string paramsStr;
        for (int i = 0; i < funcDef.params.size(); ++i) {
            if (i == 0) {
                paramsStr += funcDef.params[i].name;
            } else {
                paramsStr += ", " + funcDef.params[i].name;
            }
        }

        CodeGenerator codeGen;
        auto bodyStr = codeGen.GenCode(*funcDef.body, environment);

        code = (boost::format(FUNC_DEF_TEMPLATE) % paramsStr % bodyStr).str();
    }
    SUSHI_VISITING(ast::IfStmt, ifStmt);
    SUSHI_VISITING(ast::ReturnStmt, returnStmt);
    SUSHI_VISITING(ast::SwitchStmt, switchStmt);
    SUSHI_VISITING(ast::ForStmt, forStmt);
    SUSHI_VISITING(ast::LoopControlStmt, loopControlStmt);
    SUSHI_VISITING(ast::Expression, expression);
};

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program & program,
        const Environment & environment) {
        std::string ret;
        for (auto &statement : program.statements) {
            CodeGenStmtVisitor visitor(environment, program);
            statement->AcceptVisitor(visitor);
            ret += visitor.code + "\n";
        }
    }
};

}  // namespace sushi
