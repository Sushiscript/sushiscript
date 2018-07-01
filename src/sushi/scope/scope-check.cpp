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

std::vector<Error> ScopeCheck(const ast::Program &program, Environment &environment) {
    // initial top environment
    // Environment environment;
    // initial top scope, outer is nullptr
    std::shared_ptr<Scope> head_scope(new Scope(nullptr));
    // add the <program, scope> to environment
    environment.Insert(&program, head_scope);

    std::vector<Error> errs;

    // initial statement visitor
    StatementVisitor visitor(environment, head_scope);

    for (auto &statement : program.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor);
    }

    MergeVector(errs, visitor.errs);

    return errs;
};

} // namespace scope
} // namespace sushi
