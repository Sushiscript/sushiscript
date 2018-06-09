#include "sushi/code-generation/code-generation.h"

namespace sushi {

std::string CodeGenerator::GenCode(
    const ast::Program & program,
    const scope::Environment & environment) {
    std::string ret;
    for (auto &statement : program.statements) {
        CodeGenStmtVisitor visitor(environment, program);
        statement->AcceptVisitor(visitor);
        ret += visitor.code;
    }
    return ret;
}

};
