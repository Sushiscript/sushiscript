#ifndef SUSHI_PARSER_ERROR_H_
#define SUSHI_PARSER_ERROR_H_

#include "sushi/lexer/token.h"
#include <iostream>

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
    static std::string ToString(Type t) {
        switch (t) {
        case Type::kLexicalError: return "LexicalError";
        case Type::kEmptyBlock: return "EmptyBlock";
        case Type::kExpectToken: return "ExpectToken";
        case Type::kUnexpectIndent: return "UnexpectedIndent";
        case Type::kUnexpectToken: return "ExpectToken";
        case Type::kUnexpectedEof: return "UnexpectedEOF";
        case Type::kExpectExpression: return "ExpectExpression";
        case Type::kExpectCommand: return "ExpectCommand";
        case Type::kExpectType: return "ExpectType";
        case Type::kExpectSimpleType: return "ExpectSimpleType";
        case Type::kWrongTypeKind: return "WrongTypeKind";
        case Type::kExpectMappedValue: return "ExpectMappedValue";
        case Type::kExpectRedirItem: return "ExpectRedirectItem";
        }
    }
    std::string ToString() const {
        return '[' + Error::ToString(type) + ',' + position.ToString() + ']';
    }
    Type type;
    lexer::Token position;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_ERROR_H_
