#ifndef SUSHI_SCOPE_H_
#define SUSHI_SCOPE_H_

#include "sushi/scope/scope.h"
#include "sushi/scope/environment.h"
#include "sushi/scope/type-check.h"
#include "sushi/scope/error.h"
#include "sushi/scope/visitor/statement-visitor.h"

/// what scope do ? 
/// scope part is between the lexer part and code-generation part, it will 
/// receive infomation from lexer part and offer some useful services to 
/// code-generation part, in detail:
/// 1.type-check part, as far as I know, now we don't do the type check works.
/// 2.find-scope part, help to find {a. identifier, b. expression, c. program}'s
/// scope. 

/// more details about type check:
/// not about the grammer error, but about the scope error, fellow the docs and 
/// try to check the error like operator's variable type not match.


#endif // SUSHI_SCOPE_H_
