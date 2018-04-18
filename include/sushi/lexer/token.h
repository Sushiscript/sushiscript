#ifndef SUSHI_LEXER_TOKEN_H_
#define SUSHI_LEXER_TOKEN_H_

#include "./token-location.h"
#include "boost/variant.hpp"
#include <string>

namespace sushi {

struct Token {
    enum class Type {
        kIdent,
        // keywords
        kOr,
        kNot,
        kAnd,
        kDefine,
        kReturn,
        kExport,
        kIf,
        kElse,
        kSwitch,
        kCase,
        kDefault,
        kFor,
        kIn,
        kBreak,
        kContinue,
        kRedirect,
        kFrom,
        kTo,
        kHere,
        // built-in types
        kInt,
        kBool,
        kUnit,
        kChar,
        kString,
        kPath,
        kArray,
        kMap,
        kExitCode,
        kFd,
        // named literals
        kStdin,
        kStdout,
        kStderr,
        kUnitLit,
        kTrue,
        kFalse,
        // punctuations
        kPlus,        // +
        kMinus,       // -
        kStar,        // *
        kSlash,       // /
        kPercent,     // %
        kLAngle,      // <
        kRAngle,      // >
        kGreaterEq,   // >=
        kLessEq,      // <=
        kDoubleEq,    // ==
        kNotEqual,    // !=
        kComma,       // ,
        kSingleEq,    // =
        kColon,       // :
        kExclamation, // !
        kDollar,      // $
        // literals
        kCharLit,
        kStringLit,
        kPathLit,
        kIntLit,
        kRawString,
        // special
        kIndent,
        kLineBreak,
        kError
    };

    using Data = boost::variant<int, std::string>;

    Token::Type type;
    TokenLocation location;
    Data content;
};

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_H_
