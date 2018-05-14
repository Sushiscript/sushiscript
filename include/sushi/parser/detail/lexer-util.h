#ifndef SUSHI_PARSER_LEXER_UTIL_H_
#define SUSHI_PARSER_LEXER_UTIL_H_

#include "./token-util.h"
#include "sushi/lexer/lexer.h"
#include <functional>

namespace sushi {
namespace parser {
namespace detail {

boost::optional<const lexer::Token &>
SkipSpaceLookahead(lexer::Lexer &, int n = 1);

boost::optional<lexer::Token> SkipSpaceNext(lexer::Lexer &);

boost::optional<const lexer::Token &>
Lookahead(lexer::Lexer &, bool, int n = 1);

optional<const lexer::Token &> OptionalLookahead(
    lexer::Lexer &lex, lexer::Token::Type t, bool skip_space = true, int n = 1);

boost::optional<lexer::Token> Next(lexer::Lexer &, bool);

boost::optional<lexer::Token>
Optional(lexer::Lexer &lex, lexer::Token::Type t, bool skip_space = true);

boost::optional<lexer::Token> Optional(
    lexer::Lexer &lex, std::function<bool(lexer::Token::Type)> p,
    bool skip_space = true);

} // namespace detail
} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_LEXER_UTIL_H_