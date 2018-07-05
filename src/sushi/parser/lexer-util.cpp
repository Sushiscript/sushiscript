#include "sushi/parser/detail/lexer-util.h"
#include "sushi/parser/detail/token-util.h"

using boost::none;
using boost::optional;

namespace sushi {
namespace parser {
namespace detail {

using lexer::Lexer;
using lexer::Token;

optional<const Token &> SkipSpaceLookahead(Lexer &lex, int n) {
    int i = 1;
    for (;; ++i) {
        auto l = lex.Lookahead(i);
        if (not l) return boost::none;
        if (not IsSpace(l->type)) break;
    }
    return lex.Lookahead(i + n - 1);
}

optional<Token> SkipSpaceNext(Lexer &lex) {
    lex.SkipWhile([](const Token &t) { return IsSpace(t.type); });
    return lex.Next();
}

optional<const Token &> Lookahead(Lexer &lex, bool skip_space, int n) {
    return skip_space ? SkipSpaceLookahead(lex, n) : lex.Lookahead(n);
}

optional<const Token &>
OptionalLookahead(Lexer &lex, Token::Type t, bool skip_space, int n) {
    auto l = Lookahead(lex, skip_space, n);
    if (l and l->type == t) return l;
    return none;
}

optional<Token> Next(Lexer &lex, bool skip_space) {
    return skip_space ? SkipSpaceNext(lex) : lex.Next();
}

optional<Token> Optional(Lexer &lex, Token::Type t, bool skip_space) {
    auto l = Lookahead(lex, skip_space);
    if (l and l->type == t) return Next(lex, skip_space);
    return none;
}

optional<Token>
Optional(Lexer &lex, std::function<bool(Token::Type)> p, bool skip_space) {
    auto l = Lookahead(lex, skip_space);
    if (l and p(l->type)) return Next(lex, skip_space);
    return none;
}

Token LookaheadAsToken(Lexer &lex, bool skip_space, int n) {
    auto l = Lookahead(lex, skip_space, n);
    if (not l) return Token::Eof();
    return *l;
}

} // namespace detail
} // namespace parser
} // namespace sushi
