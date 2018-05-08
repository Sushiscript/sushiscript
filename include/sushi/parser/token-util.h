#ifndef SUSHI_PARSER_TOKEN_UTIL_H_
#define SUSHI_PARSER_TOKEN_UTIL_H_

#include "sushi/lexer/token.h"
#include "sushi/type-system/type.h"

namespace sushi {
namespace parser {

bool IsBinaryOp(lexer::Token::Type);

int BinaryOpPrec(lexer::Token::Type);

bool IsUnaryOp(lexer::Token::Type);

bool IsType(lexer::Token::Type);

type::BuiltInAtom::Type TypeTokenToType(lexer::Token::Type);

bool IsLiteral(lexer::Token::Type);

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_TOKEN_UITL_H_