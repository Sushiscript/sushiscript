#ifndef SUSHI_PARSER_ERROR_H_
#define SUSHI_PARSER_ERROR_H_

#include "sushi/lexer/token.h"

namespace sushi {
namespace parser {

struct Error {
    enum class Type {
        kLexicalError,
        kEmptyBlock,
        kExpectToken,
        kUnexpectIndent,
        kUnexpectToken,
        kUnexpectedEof,
        kExpectExpression,
        kExpectCommand,
        kExpectType,
        kExpectSimpleType,
        kWrongTypeKind,
        kExpectMappedValue,
        kExpectRedirItem
    };
    Type type;
    lexer::Token position;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_ERROR_H_
