#include "sushi/code-generation/code-generation.h"
#include "sushi/code-generation/stmt-visitor.h"

namespace sushi {

std::string CodeGenerator::GenCode(
    const ast::Program & program,
    const scope::Environment & environment,
    std::shared_ptr<ScopeManager> scope_manager) {
    if (!scope_manager) {
        scope_manager = std::make_shared<ScopeManager>(new ScopeManager());
    }
    std::string ret;
    for (auto &statement : program.statements) {
        CodeGenStmtVisitor visitor(environment, program, scope_manager);
        statement->AcceptVisitor(visitor);
        ret += visitor.code + "\n";
    }
    return ret;
}

};
