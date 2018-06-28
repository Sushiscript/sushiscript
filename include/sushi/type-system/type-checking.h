#ifndef SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_
#define SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_

#include "type-check/state.h"
#include "type-check/statement.h"
#include <vector>

namespace sushi {

namespace type {

std::vector<Error> Check(const ast::Program &program, Environment &env) {
    State::ScopeBindings bindings;
    std::vector<Error> errors;
    CheckProgram(State(
        bindings, program, env, errors,
        BuiltInAtom::Make(BuiltInAtom::Type::kExitCode)));
    return errors;
}

// struct Checker {
//     std::vector<Error> Check(const ast::Program &program, Environment &env);

//     State::ScopeBindings bindings;
// };

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_