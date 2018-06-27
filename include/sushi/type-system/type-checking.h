#ifndef SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_
#define SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_

#include "./error.h"
#include "sushi/ast.h"
#include "sushi/scope.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace sushi {

namespace type {

struct Checker {
    std::vector<Error> Check(const Program &program, Environment &env);

    std::unique_ptr<Type> VariableType(const Identifier &ident) {
        auto scope = env->LookUp(&ident);
        return name_table[scope][ident.name];
    }

    std::unordered_map<const Scope *, std::unordered_map<std::string, Type *>>
        name_table;

    Environment *env = nullptr;
};

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_CHECKING_H_