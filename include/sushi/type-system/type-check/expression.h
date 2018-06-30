#ifndef SUSHI_TYPE_SYSTEM_TYPE_CHECK_EXPRESSION_H_
#define SUSHI_TYPE_SYSTEM_TYPE_CHECK_EXPRESSION_H_

#include "sushi/ast.h"
#include "sushi/type-system/type.h"
#include "sushi/type-system/type-check/state.h"
#include <functional>

namespace sushi {

namespace type {

struct DeduceResult {
    DeduceResult(
        Type::Pointer type = nullptr, bool empty_array = false, bool fail = false)
        : type(std::move(type)), empty_array(empty_array), fail(fail) {}

    static DeduceResult Fail() {
        return {nullptr, false, true};
    }
    static DeduceResult EmptyArray() {
        return {nullptr, true, false};
    }

    Type::Pointer type;
    bool empty_array = false;
    bool fail = false;
};

DeduceResult Deduce(const ast::Expression &expr, State &state);

bool SatisfyRequirement(
    const ast::Expression &expr, Type::Pointer should_be, State &state);

Type::Pointer RequireOneOf(
    const ast::Expression &expr, State &state, std::vector<Type::Pointer> ts);

Type::Pointer RequireSatisfy(
    const ast::Expression &expr, State &state,
    std::function<Type::Pointer(const DeduceResult &)> pred);

Type::Pointer
UnambiguousDeduce(const ast::Expression &expr, State &s, bool insert = true);

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_CHECK_EXPRESSION_H_