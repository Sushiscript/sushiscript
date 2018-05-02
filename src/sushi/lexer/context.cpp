#include "sushi/lexer/context.h"
#include "sushi/lexer/detail/character-config.h"
#include "sushi/lexer/error.h"
#include "sushi/lexer/token.h"
#include "sushi/util/optional.h"
#include <cctype>

namespace sushi {
namespace lexer {

using boost::none;
using boost::optional;
using detail::LexerState;

namespace {

bool isindenthead(char c) {
    return static_cast<bool>(isalpha(c)) or c == '_';
}

auto RecordLocation(const LexerState &s) {
    return [l = s.input.NextLocation()](Token::Type t, Token::Data d) mutable {
        return Token{t, std::move(l), std::move(d)};
    };
}

Token SkipAndMake(LexerState &s, Token::Type t, int n = 1, Token::Data d = 0) {
    auto l = s.input.NextLocation();
    s.input.Skip(n);
    return Token{t, std::move(l), std::move(d)};
}

namespace unsafe {

Token LineBreak(LexerState &s) {
    s.line_start = true;
    return SkipAndMake(s, Token::Type::kLineBreak);
}

}; // namespace unsafe

namespace try_ {

bool JoinLine(LexerState &s) {
    if (s.input.LookaheadMany(2) == "\\\n") {
        s.input.Take(2);
        return true;
    }
    return false;
}

optional<Token> LineBreak(LexerState &s) {
    auto c = s.input.Lookahead();
    if (not c or *c != '\n') return none;
    return unsafe::LineBreak(s);
}

bool LineComment(LexerState &s) {
    auto oc = s.input.Lookahead();
    if (oc and *oc == '#') {
        s.input.SkipWhile([](char c) { return c != '\n'; });
        return true;
    }
    return false;
}

optional<Token> DoubleOperator(LexerState &s) {
    std::string op = s.input.LookaheadMany(2);
    if (op.size() < 2) return none;
    auto iter = Token::DoublePunctuationMap().find(op);
    if (iter == end(Token::DoublePunctuationMap())) {
        return none;
    }
    return SkipAndMake(s, iter->second, 2);
}
optional<Token> SingleOperator(LexerState &s) {
    auto iter = Token::SinglePunctuationMap().find(*s.input.Lookahead());
    if (iter == end(Token::SinglePunctuationMap())) {
        return none;
    }
    return SkipAndMake(s, iter->second, 1);
}
optional<Token> Punctuation(LexerState &s) {
    using namespace sushi::util::monadic_optional;
    return DoubleOperator(s) | [&s]() { return SingleOperator(s); };
}

} // namespace try_

void SkipSpaces(LexerState &s) {
    s.input.SkipWhile([](char c) { return c == ' '; });
    if (try_::JoinLine(s)) {
        SkipSpaces(s);
    }
}

optional<char> Character(LexerState &s, const detail::CharacterConfig &cc) {
    using namespace sushi::util::monadic_optional;
    auto &input = s.input;
    return input.Lookahead(1) >> [&](char c1) -> optional<char> {
        if (c1 == '\\') {
            return input.Lookahead(2) >> [&](char c2) -> optional<char> {
                return cc.Escape(c2) >> [&](char ce) -> optional<char> {
                    input.Skip(2);
                    return ce;
                };
            };
        }
        auto oc = cc.Unescape(c1);
        if (oc) input.Skip(1);
        return oc;
    };
}

std::string String(LexerState &s, const detail::CharacterConfig &cc) {
    std::string str;
    for (boost::optional<char> n; (n = Character(s, cc));) str.push_back(*n);
    return str;
}

namespace unsafe {

Token Indentation(LexerState &s) {
    auto token = RecordLocation(s);
    std::string indent = s.input.TakeWhile([](char c) { return c == ' '; });
    if (try_::JoinLine(s) or static_cast<bool>(try_::LineBreak(s))) {
        return Indentation(s);
    }
    s.line_start = false;
    return token(Token::Type::kIndent, indent.size());
}

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

Token CharLiteral(LexerState &s) {
    auto token = RecordLocation(s);
    auto &input = s.input;
    input.Next();
    std::string str = String(s, detail::CharConfig());
    auto tail = s.input.Lookahead();
    if (not tail or *tail != '\'') {
        return token(
            Token::Type::kErrorCode,
            static_cast<int>(Error::kUnclosedCharQuote));
    }
    input.Next();
    if (str.size() == 1) return token(Token::Type::kCharLit, str.front());
    return token(
        Token::Type::kErrorCode, static_cast<int>(Error::kInvalidChar));
}

Token IntLiteral(LexerState &s) {
    auto token = RecordLocation(s);
    auto i = s.input.TakeWhile([](char c) { return std::isdigit(c); });
    return token(Token::Type::kIntLit, std::stoi(i));
}

Token Identifier(LexerState &s) {
    auto &input = s.input;
    auto loc = input.NextLocation();
    std::string head(1, *input.Next());
    std::string rest =
        input.TakeWhile([](char c) { return isalnum(c) or c == '_'; });
    return Token::FromIdent(std::move(head) + std::move(rest), std::move(loc));
}

Token UnknownCharacter(LexerState &s) {
    char c = *s.input.Lookahead();
    return SkipAndMake(s, Token::Type::kOtherChar, 1, c);
}

Token StartInterpolation(LexerState &s) {
    auto token = RecordLocation(s);
    auto &input = s.input;
    input.Next();
    auto oc = input.Lookahead();
    if (not oc)
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kSingleDollar));
    if (isindenthead(*oc)) return unsafe::Identifier(s);
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

Token NormalDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '\'') return unsafe::CharLiteral(s);
    if (isdigit(lookahead)) return unsafe::IntLiteral(s);
    if (isindenthead(lookahead)) return unsafe::Identifier(s);
    if (boost::optional<Token> punct = try_::Punctuation(s)) return *punct;
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return unsafe::PathLiteral(s);
    return unsafe::UnknownCharacter(s);
}

Token RawDispatch(LexerState &s) {
    char lookahead = *s.input.Lookahead();
    if (lookahead == '$') return unsafe::StartInterpolation(s);
    if (lookahead == '"') return unsafe::StringLiteral(s);
    if (lookahead == '\'') return unsafe::CharLiteral(s);
    if (lookahead == ';') return SkipAndMake(s, Token::Type::kSemicolon);
    if (lookahead == '.' or lookahead == '/' or lookahead == '~')
        return unsafe::PathLiteral(s);
    return unsafe::RawToken(s);
}

boost::optional<Token> StartOfLine(NormalContext *ctx) {
    auto &s = ctx->state;
    auto indent = unsafe::Indentation(s);
    if (not s.input.Lookahead()) return none;
    if (try_::LineComment(s)) {
        unsafe::LineBreak(s);
        return ctx->Lex();
    }
    return indent;
}

} // namespace

boost::optional<Token> NormalContext::Lex() {
    if (state.line_start) return StartOfLine(this);
    SkipSpaces(state);
    try_::LineComment(state);
    if (not state.input.Lookahead()) return none;
    if (optional<Token> t = try_::LineBreak(state)) return *t;
    return NormalDispatch(state);
}

boost::optional<Token> RawContext::Lex() {
    SkipSpaces(state);
    try_::LineComment(state);
    if (not state.input.Lookahead()) return none;
    if (optional<Token> t = try_::LineBreak(state)) return *t;
    return RawDispatch(state);
}

boost::optional<Token> StringLiteralContext::Lex() {
    return boost::none;
}

boost::optional<Token> RawLiteralContext::Lex() {
    return boost::none;
}

} // namespace lexer
} // namespace sushi