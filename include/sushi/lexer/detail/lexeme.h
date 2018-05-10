#ifndef SUSHI_LEXER_DETAIL_LEXEME_H_
#define SUSHI_LEXER_DETAIL_LEXEME_H_

#include "./lexer-state.h"
#include "boost/optional.hpp"
#include "sushi/lexer/detail/character-config.h"
#include "sushi/lexer/token.h"
#include <cctype>

namespace sushi {
namespace lexer {
namespace detail {

inline bool isindenthead(char c) {
    return static_cast<bool>(isalpha(c)) or c == '_';
}

inline auto RecordLocation(const LexerState &s) {
    return [l = s.input.NextLocation()](Token::Type t, Token::Data d) mutable {
        return Token{t, std::move(l), std::move(d)};
    };
}

Token SkipAndMake(LexerState &s, Token::Type t, int n = 1, Token::Data d = 0);

Token UnsafeLineBreak(LexerState &s);

bool TryJoinLine(LexerState &s);

boost::optional<Token> TryLineBreak(LexerState &s);

bool TryLineComment(LexerState &s);

boost::optional<Token> Punctuation(LexerState &s);

void SkipSpaces(LexerState &s);

boost::optional<char>
Character(LexerState &s, const detail::CharacterConfig &cc);

std::string String(LexerState &s, const detail::CharacterConfig &cc);

Token Indentation(LexerState &s);

Token IntLiteral(LexerState &s);

Token CharLiteral(LexerState &s);

Token Identifier(LexerState &s);

Token UnknownCharacter(LexerState &s);

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_LEXEME_H_