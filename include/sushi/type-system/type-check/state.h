#ifndef SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATE_H_
#define SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATE_H_

#include "sushi/ast.h"
#include "sushi/scope.h"
#include "sushi/type-system/type-check/error.h"

namespace sushi {

namespace type {

struct State {
    using ScopeBindings = std::unordered_map<
        const Scope *, std::unordered_map<std::string, Type::Pointer>>;
    State(
        ScopeBindings &bindings, const ast::Program &program, Environment &env,
        std::vector<Error> &errors, Type::Pointer return_type)
        : bindings(bindings), program(program), env(env), errors(errors),
          return_type(std::move(return_type)) {}

    void InsertName(const std::string &name, Type::Pointer tp) {
        auto def_scope = env.LookUp(&program);
        bindings[def_scope][name] = std::move(tp);
    }
    Type::Pointer LookupName(const ast::Identifier &ident) {
        auto def_scope = env.LookUp(&ident)->LookUp(ident.name)->defined_scope;
        auto &p = bindings[def_scope][ident.name];
        if (p) return p->Copy();
        return nullptr;
    }
    State FromNewProgram(const ast::Program &p) {
        return State(bindings, p, env, errors, return_type->Copy());
    }
    State NewFunctionBody(const ast::Program &p, Type::Pointer new_ret) {
        return State(bindings, p, env, errors, std::move(new_ret));
    }
    Type::Pointer TypeError(Error e) {
        errors.push_back(std::move(e));
        return nullptr;
    }
    Type::Pointer TypeError(const ast::Expression *expr, Error::Tp t) {
        errors.emplace_back(expr, t);
        return nullptr;
    }

    ScopeBindings &bindings;
    const ast::Program &program;
    Environment &env;
    std::vector<Error> &errors;
    Type::Pointer return_type;
};

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATE_H_