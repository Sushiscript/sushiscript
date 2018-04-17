#ifndef SUSHI_LEXER_TOKEN_H_
#define SUSHI_LEXER_TOKEN_H_

#include "./token-location.h"
#include <string>

namespace sushi {

struct Token {
    enum class Type {
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
        // special
        kIndent,
        kLineBreak,
        kError
    };

    Token::Type type;
    TokenLocation location;
    std::string content = "";
};

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_H_
