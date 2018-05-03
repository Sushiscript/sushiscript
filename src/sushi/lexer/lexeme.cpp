#include "sushi/lexer/detail/lexeme.h"
#include "sushi/util/optional.h"
#include "sushi/lexer/error.h"

using boost::optional;
using boost::none;

namespace sushi {
namespace lexer {
namespace detail {

namespace {


optional<Token> TryDoubleOperator(LexerState &s) {
    std::string op = s.input.LookaheadMany(2);
    if (op.size() < 2) return none;
    auto iter = Token::DoublePunctuationMap().find(op);
    if (iter == end(Token::DoublePunctuationMap())) {
        return none;
    }
    return SkipAndMake(s, iter->second, 2);
}
optional<Token> TrySingleOperator(LexerState &s) {
    auto iter = Token::SinglePunctuationMap().find(*s.input.Lookahead());
    if (iter == end(Token::SinglePunctuationMap())) {
        return none;
    }
    return SkipAndMake(s, iter->second, 1);
}

}

Token SkipAndMake(LexerState &s, Token::Type t, int n, Token::Data d) {
    auto l = s.input.NextLocation();
    s.input.Skip(n);
    return {t, std::move(l), std::move(d)};
}

bool TryJoinLine(LexerState &s) {
    if (s.input.LookaheadMany(2) == "\\\n") {
        s.input.Take(2);
        return true;
    }
    return false;
}

Token UnsafeLineBreak(LexerState &s) {
    s.line_start = true;
    return SkipAndMake(s, Token::Type::kLineBreak);
}

optional<Token> TryLineBreak(LexerState &s) {
    auto c = s.input.Lookahead();
    if (not c or *c != '\n') return none;
    return UnsafeLineBreak(s);
}

bool TryLineComment(LexerState &s) {
    auto oc = s.input.Lookahead();
    if (oc and *oc == '#') {
        s.input.SkipWhile([](char c) { return c != '\n'; });
        return true;
    }
    return false;
}

optional<Token> Punctuation(LexerState &s) {
    using namespace sushi::util::monadic_optional;
    return TryDoubleOperator(s) | [&s]() { return TrySingleOperator(s); };
}

void SkipSpaces(LexerState &s) {
    s.input.SkipWhile([](char c) { return c == ' '; });
    if (TryJoinLine(s)) {
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

Token Indentation(LexerState &s) {
    auto token = RecordLocation(s);
    std::string indent = s.input.TakeWhile([](char c) { return c == ' '; });
    if (TryJoinLine(s) or static_cast<bool>(TryLineBreak(s))) {
        return Indentation(s);
    }
    s.line_start = false;
    return token(Token::Type::kIndent, indent.size());
}

Token IntLiteral(LexerState &s) {
    auto token = RecordLocation(s);
    auto i = s.input.TakeWhile([](char c) { return std::isdigit(c); });
    return token(Token::Type::kIntLit, std::stoi(i));
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
    return SkipAndMake(s, Token::Type::kUnknownChar, 1, c);
}

} // namespace detail
} // namespace lexer
} // namespace sushi