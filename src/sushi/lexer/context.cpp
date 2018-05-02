#include "sushi/lexer/context.h"
#include "sushi/lexer/detail/character-config.h"
#include "sushi/lexer/error.h"
#include "sushi/lexer/token.h"
#include "sushi/lexer/detail/lexeme.h"

namespace sushi {
namespace lexer {

using boost::none;
using boost::optional;
using LexResult = Context::LexResult;

using namespace detail;

namespace {

namespace unsafe {

Token StringLiteral(LexerState &s) {
    auto token = RecordLocation(s);
    s.input.Next();
    std::string data = String(s, detail::StringConfig());
    auto tail = s.input.Lookahead();
    if (not tail or *tail != '"') {
        return token(
            Token::Type::kErrorCode,
            static_cast<int>(Error::kUnclosedStringQuote));
    }
    s.input.Next();
    return token(Token::Type::kStringLit, data);
}

Token RawToken(LexerState &s) {
    auto token = RecordLocation(s);
    std::string tok = String(s, detail::RawConfig());
    if (tok.empty())
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kExpectRawToken));
    return token(Token::Type::kRawString, tok);
}

Token PathLiteral(LexerState &s) {
    auto token = RecordLocation(s);
    std::string path;
    if (*s.input.Lookahead() == '~') {
        path.push_back('~');
        s.input.Next();
    } else {
        path += s.input.TakeWhile([](char c) { return c == '.'; });
    }
    auto slash = RecordLocation(s);
    std::string rest = String(s, detail::RawConfig());
    if (not rest.empty() and rest.front() != '/') {
        return slash(
            Token::Type::kErrorCode, static_cast<int>(Error::kPathExpectSlash));
    }
    path += rest;
    return token(Token::Type::kPathLit, path);
}

Token StartInterpolation(LexerState &s) {
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
        return SkipAndMake(s, Token::Type::kInterStart);
    }
    if (isspace(*oc))
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kSingleDollar));
    return SkipAndMake(s, Token::Type::kInvalidChar, 1, int(*oc));
}

} // namespace unsafe

Context::LexResult NormalDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    // if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '"') return Context::SkipTransfer<StringLitContext>(s);
    if (lookahead == '\'') return CharLiteral(s);
    if (isdigit(lookahead)) return IntLiteral(s);
    if (isindenthead(lookahead)) return Identifier(s);
    if (boost::optional<Token> punct = TryPunctuation(s)) return *punct;
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::Transfer<PathLitContext>();
    return UnknownCharacter(s);
}

Context::LexResult RawDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    if (lookahead == '$') return unsafe::StartInterpolation(s);
    if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '\'') return CharLiteral(s);
    if (lookahead == ';') return SkipAndMake(s, Token::Type::kSemicolon);
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return Context::Transfer<PathLitContext>();
    return unsafe::RawToken(s);
}

} // namespace

LexResult NormalContext::StartOfLine() {
    auto indent = Indentation(state);
    if (not state.input.Lookahead()) return none;
    if (TryLineComment(state)) {
        UnsafeLineBreak(state);
        return Lex();
    }
    return std::move(indent);
}

LexResult NormalContext::Lex() {
    if (state.line_start) return StartOfLine();
    SkipSpaces(state);
    TryLineComment(state);
    if (not state.input.Lookahead()) return none;
    if (optional<Token> t = LineBreak(state)) return std::move(*t);
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

LexResult StringLitContext::Lex() {
    return boost::none;
}

LexResult RawTokenContext::Lex() {
    return boost::none;
}

LexResult PathLitContext::Lex() {
    return boost::none;
}

} // namespace lexer
} // namespace sushi