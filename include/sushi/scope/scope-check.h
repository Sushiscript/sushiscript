#ifndef SUSHI_SCOPE_TYPE_CHECK_H
#define SUSHI_SCOPE_TYPE_CHECK_H

#include "environment.h"
#include "scope.h"
#include "sushi/ast/statement.h"

namespace sushi {
namespace scope {

Environment ScopeCheck(ast::Program &program);

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_TYPE_CHECK_H
