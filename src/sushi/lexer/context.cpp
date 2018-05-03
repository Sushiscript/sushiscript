#include "sushi/lexer/context.h"
#include "sushi/lexer/detail/character-config.h"
#include "sushi/lexer/detail/lexeme.h"
#include "sushi/lexer/error.h"
#include "sushi/lexer/token.h"

namespace sushi {
namespace lexer {

using boost::none;
using boost::optional;
using LexResult = Context::LexResult;

using namespace detail;

namespace {

LexResult NormalDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    // if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '\n') return UnsafeLineBreak(s);
    if (lookahead == '"')
        return Context::EmitEnter<StringLitContext>(
            SkipAndMake(s, Token::Type::kStringLit));
    if (lookahead == '\'') return CharLiteral(s);
    if (isdigit(lookahead)) return IntLiteral(s);
    if (isindenthead(lookahead)) return Identifier(s);
    if (boost::optional<Token> punct = Punctuation(s)) return *punct;
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::EmitEnter<PathLitContext>(
            Token{Token::Type::kPathLit, s.input.NextLocation(), 0});
    return UnknownCharacter(s);
}

Context::LexResult RawDispatch(LexerState &s) {
    auto token = RecordLocation(s);
    char lookahead = *s.input.Lookahead();
    if (lookahead == '\n') return UnsafeLineBreak(s);
    if (lookahead == '"')
        return Context::EmitEnter<StringLitContext>(
            SkipAndMake(s, Token::Type::kStringLit));
    if (lookahead == '\'') return CharLiteral(s);
    if (lookahead == ';') return SkipAndMake(s, Token::Type::kSemicolon);
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::EmitEnter<PathLitContext>(
            token(Token::Type::kPathLit, 0));
    if (RawConfig().Prohibit(lookahead))
        return SkipAndMake(s, Token::Type::kUnknownChar, 1, lookahead);
    return Context::EmitEnter<RawTokenContext>(
        token(Token::Type::kRawString, 0));
}

} // namespace

LexResult NormalContext::StartOfLine() {
    auto indent = Indentation(state);
    if (not state.input.Lookahead()) return none;
    if (TryLineComment(state)) {
        UnsafeLineBreak(state);
        return StartOfLine();
    }
    return std::move(indent);
}

LexResult NormalContext::Lex() {
    if (state.line_start) return StartOfLine();
    SkipSpaces(state);
    TryLineComment(state);
    if (not state.input.Lookahead()) return none;
    return NormalDispatch(state);
}

LexResult RawContext::Lex() {
    state.line_start = false;
    SkipSpaces(state);
    TryLineComment(state);
    if (not state.input.Lookahead()) return none;
    if (optional<Token> t = TryLineBreak(state))
        return LexResult(std::move(*t));
    return RawDispatch(state);
}

} // namespace lexer
} // namespace sushi