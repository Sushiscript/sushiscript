#ifndef SUSHI_TYPE_SYSTEM_ERROR_H_
#define SUSHI_TYPE_SYSTEM_ERROR_H_

#include "sushi/ast/expression.h"
#include "sushi/ast/to-string.h"

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
    Error(const ast::Expression *expr, Tp type) : expr(expr), type(type) {}

    static std::string ToString(Error::Tp t) {
        switch (t) {
        case kAmbiguousType: return "AmbiguousType";
        case kInvalidRange: return "InvalidRange";
        case kInvalidType: return "InvalidType";
        case kInvalidLvalue: return "InvalidLvalue";
        case kMissingReturnValue: return "MissingReturnValue";
        case kRequireSimpleType: return "RequireSimpleType";
        case kInvalidIndexable: return "InvalidIndexable";
        case kInvalidFunction: return "InvalidFunction";
        case kWrongNumOfParams: return "WrongNumOfParams";
        }
    }
    std::string ToString() const {
        std::string result = ToString(type);
        if (expr != nullptr) result += ": " + ast::ToString(expr);
        return result;
    }
    const ast::Expression *expr;
    Tp type;
};

} // namespace type

} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_ERROR_H_