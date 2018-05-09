#ifndef SUSHI_PARSER_LEXER_UTIL_H_
#define SUSHI_PARSER_LEXER_UTIL_H_

#include "sushi/lexer/lexer.h"
#include "./token-util.h"

namespace sushi {
namespace parser {
namespace detail {


boost::optional<const lexer::Token &> SkipSpaceLookahead(lexer::Lexer&);

boost::optional<lexer::Token> SkipSpaceNext(lexer::Lexer&);

boost::optional<const lexer::Token &> Lookahead(lexer::Lexer&, bool);

boost::optional<lexer::Token> Next(lexer::Lexer&, bool);

boost::optional<lexer::Token> Optional(
    lexer::Lexer& lex, lexer::Token::Type t, bool skip_space = true);

} // namespace detail
} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_LEXER_UTIL_H_