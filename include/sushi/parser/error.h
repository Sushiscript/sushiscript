#ifndef SUSHI_PARSER_ERROR_H_
#define SUSHI_PARSER_ERROR_H_

#include "sushi/lexer/token.h"

namespace sushi {
namespace parser {

struct Error {
    enum class Type {
        kLexicalError,
        kExpectToken,
        kUnexpectIndent,
        kUnexpectToken,
        kUnexpectedEof,
        kExpectCommand,
        kExpectType,
        kExpectSimpleType,
        kWrongTypeKind,
        kExpectMappedValue
    };
    Type type;
    lexer::Token position;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_ERROR_H_
