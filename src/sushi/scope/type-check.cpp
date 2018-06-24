#ifndef SUSHI_SCOPE_TYPE_CHECK_H
#define SUSHI_SCOPE_TYPE_CHECK_H

#include "sushi/ast.h"
#include "sushi/scope.h"
#include <iostream>

namespace sushi {
namespace scope {

Environment TypeCheck(ast::Program & program) {
    Environment environment;
    std::shared_ptr<Scope> head_scope (new Scope (nullptr));
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
