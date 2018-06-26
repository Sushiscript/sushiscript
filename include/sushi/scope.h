#ifndef SUSHI_SCOPE_H_
#define SUSHI_SCOPE_H_

#include "sushi/scope/environment.h"
#include "sushi/scope/error.h"
#include "sushi/scope/scope.h"
#include "sushi/scope/type-check.h"
#include "sushi/scope/visitor/command-like-visitor.h"
#include "sushi/scope/visitor/expression-visitor.h"
#include "sushi/scope/visitor/literal-visitor.h"
#include "sushi/scope/visitor/statement-visitor.h"
#include "sushi/scope/visitor/type-visitor.h"

/// what scope do ?
/// scope part is between the lexer part and code-generation part, it will
/// receive infomation from lexer part and offer some useful services to
/// code-generation part, in detail:
/// 1.type-check part, and maintail a type dict
/// 2.find-scope part, help to find {a. identifier, b. expression, c. program}'s
/// scope.

#endif // SUSHI_SCOPE_H_
