#ifndef SUSHI_TYPE_SYSTEM_ERROR_H_
#define SUSHI_TYPE_SYSTEM_ERROR_H_

#include "sushi/ast/expression.h"

namespace sushi {

namespace type {

struct Error {
    enum Tp {
        kAmbiguousType,
        kInvalidRange,
        kInvalidType,
        kInvalidLvalue,
        kMissingReturnValue,
        kRequireSimpleType,
        kInvalidIndexable,
        kInvalidFunction,
        kWrongNumOfParams
    };
    const ast::Expression* expr;
    Tp type;
};

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_ERROR_H_