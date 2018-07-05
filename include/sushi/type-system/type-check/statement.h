#ifndef SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATEMENT_H_
#define SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATEMENT_H_

#include "sushi/type-system/type.h"
#include "sushi/type-system/type-check/state.h"

namespace sushi {

namespace type {

void CheckProgram(State&& state);

void CheckProgram(State& state);

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_CHECK_STATEMENT_H_