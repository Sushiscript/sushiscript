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

namespace unsafe {

LexResult StartInterpolation(LexerState &s) {
    auto token = RecordLocation(s);
    auto &input = s.input;
    input.Next();
    auto oc = input.Lookahead();
    if (not oc)
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kSingleDollar));
    if (isindenthead(*oc)) return Identifier(s);
    if (*oc == '{') {
        input.Next();
        return LexResult(
            token(Token::Type::kInterStart, 0), NormalContext::Factory);
    }
    if (isspace(*oc))
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kSingleDollar));
    return SkipAndMake(s, Token::Type::kInvalidChar, 1, int(*oc));
}

} // namespace unsafe

LexResult NormalDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    // if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '\n') return UnsafeLineBreak(s);
    if (lookahead == '"') return Context::SkipTransfer<StringLitContext>(s);
    if (lookahead == '\'') return CharLiteral(s);
    if (isdigit(lookahead)) return IntLiteral(s);
    if (isindenthead(lookahead)) return Identifier(s);
    if (boost::optional<Token> punct = Punctuation(s)) return *punct;
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::Transfer<PathLitContext>();
    return UnknownCharacter(s);
}

Context::LexResult RawDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    if (lookahead == '\n') return UnsafeLineBreak(s);
    if (lookahead == '$') return unsafe::StartInterpolation(s);
    if (lookahead == '"') return Context::Transfer<StringLitContext>();
    if (lookahead == '\'') return CharLiteral(s);
    if (lookahead == ';') return SkipAndMake(s, Token::Type::kSemicolon);
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::Transfer<PathLitContext>();
    return Context::Transfer<RawTokenContext>();
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
    SkipSpaces(state);
    TryLineComment(state);
    if (not state.input.Lookahead()) return none;
    if (optional<Token> t = TryLineBreak(state))
        return LexResult(std::move(*t));
    return RawDispatch(state);
}

} // namespace lexer
} // namespace sushi