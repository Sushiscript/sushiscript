#ifndef SUSHI_SCOPE_TYPE_CHECK_H
#define SUSHI_SCOPE_TYPE_CHECK_H

#include "sushi/ast.h"
#include "sushi/scope.h"
#include <iostream>

namespace sushi {
namespace scope {

/*
 * Main Interface of Scope Part
 *
 * type-check will be done in the travel of the ast tree, you will finally get a
 * environment which contain the map of the scope and the type, if have error, a
 * error message will be print but will still return the environment.
 *
 */

Environment TypeCheck(ast::Program &program) {
    // initial top environment
    Environment environment;
    // initial top scope, outer is nullptr
    std::shared_ptr<Scope> head_scope(new Scope(nullptr));
    // add the <program, scope> to environment
    environment.Insert(&program, head_scope);
    // initial statement visitor
    StatementVisitor visitor(environment, head_scope);

    for (auto &statement : program.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor);
    }

    return environment;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_TYPE_CHECK_H
