#ifndef SUSHI_SCOPE_TYPE_CHECK_H
#define SUSHI_SCOPE_TYPE_CHECK_H

#include "sushi/ast.h"
#include "sushi/scope.h"

namespace sushi {
namespace scope {

Environment TypeChck(ast::Program & program) {
    Environment environment;
    std::shared_ptr<Scope> head_scope (nullptr);
    environment.Insert(&program, head_scope);

    for (auto &statement : program.statements) {
        StatementVisitor visitor(environment);
        statement->AcceptVisitor(visitor);
    }
    return environment;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_TYPE_CHECK_H
