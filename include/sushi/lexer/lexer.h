#ifndef SUSHI_LEXER_LEXER_H_
#define SUSHI_LEXER_LEXER_H_

#include "./detail/character-config.h"
#include "./detail/lookahead-stream.h"
#include "./error.h"
#include "./token.h"
#include "sushi/util/optional.h"
#include <cctype>
#include <iostream>
#include <istream>
#include <string>
#include <utility>

namespace sushi {
namespace lexer {

namespace detail {

inline bool IsIdentHead(char c) {
    return static_cast<bool>(isalpha(c)) or c == '_';
}

class SourceStream : public LookaheadStream<char> {
  public:
    SourceStream(std::istream &is, TokenLocation start)
        : is_(is), next_loc_(std::move(start)) {}

    boost::optional<char> Next() override {
        auto n = LookaheadStream<char>::Next();
        if (n) {
            if (*n == '\n')
                next_loc_.NewLine();
            else
                next_loc_.NextColumn();
        }
        return n;
    }

    const TokenLocation &NextLocation() const {
        return next_loc_;
    }

  private:
    boost::optional<char> Consume() override {
        if (is_.eof()) return boost::none;
        auto c = is_.get();
        return std::istream::traits_type::not_eof(c)
                   ? boost::make_optional<char>(static_cast<char>(c))
                   : boost::none;
    }

    std::istream &is_;
    TokenLocation next_loc_;
};

} // namespace detail

class Lexer : public detail::LookaheadStream<Token> {
  public:
    Lexer(std::istream &is, TokenLocation start) : input_(is, start) {}

    void Raw(bool b) {
        raw_mode_ = b;
    }
    bool Raw() {
        return raw_mode_;
    }

  private:
    auto RecordLocation() {
        return [loc = input_.NextLocation()](
                   Token::Type t, Token::Data s) mutable {
            return Token{t, std::move(loc), std::move(s)};
        };
    }
    Token SkipAndMake(Token::Type t, int n = 1, Token::Data d = 0) {
        auto location = input_.NextLocation();
        input_.Skip(n);
        return Token{t, std::move(location), std::move(d)};
    }
    Token Indentation() {
        auto token = RecordLocation();
        std::string indent = input_.TakeWhile([](char c) { return c == ' '; });
        if (TryJoinLine() or static_cast<bool>(TryLineBreak())) {
            return Indentation();
        }
        start_of_line_ = false;
        return token(Token::Type::kIndent, indent.size());
    }
    Token LineBreak() {
        // assert(*input_.Lookahead() == '\n')
        start_of_line_ = true;
        return SkipAndMake(Token::Type::kLineBreak);
    }

    boost::optional<Token> TryLineBreak() {
        auto c = input_.Lookahead();
        if (not c or *c != '\n') return boost::none;
        return LineBreak();
    }
    bool TryLineComment() {
        auto oc = input_.Lookahead();
        if (oc and *oc == '#') {
            input_.SkipWhile([](char c) { return c != '\n'; });
            return true;
        }
        return false;
    }
    bool TryJoinLine() {
        if (input_.LookaheadMany(2) == "\\\n") {
            input_.Take(2);
            return true;
        }
        return false;
    }
    boost::optional<Token> TryDoubleOperator() {
        std::string op = input_.LookaheadMany(2);
        if (op.size() < 2) return boost::none;
        auto iter = Token::DoublePunctuationMap().find(op);
        if (iter == end(Token::DoublePunctuationMap())) {
            return boost::none;
        }
        return SkipAndMake(iter->second, 2);
    }
    boost::optional<Token> TrySingleOperator() {
        auto iter = Token::SinglePunctuationMap().find(*input_.Lookahead());
        if (iter == end(Token::SinglePunctuationMap())) {
            return boost::none;
        }
        return SkipAndMake(iter->second, 1);
    }
    boost::optional<Token> TryPunctuation() {
        using namespace sushi::util::monadic_optional;
        return TryDoubleOperator() | [this]() { return TrySingleOperator(); };
    }
    void SkipSpaces() {
        input_.SkipWhile([](char c) { return c == ' '; });
        if (TryJoinLine()) {
            SkipSpaces();
        }
    }
    boost::optional<char> Character(const detail::CharacterConfig &cc) {
        using namespace sushi::util::monadic_optional;
        using boost::optional;
        return input_.Lookahead(1) >> [&](char c1) -> optional<char> {
            if (c1 == '\\') {
                return input_.Lookahead(2) >> [&](char c2) -> optional<char> {
                    return cc.Escape(c2) >> [&](char ce) -> optional<char> {
                        input_.Skip(2);
                        return ce;
                    };
                };
            }
            auto oc = cc.Unescape(c1);
            if (oc) input_.Skip(1);
            return oc;
        };
    }
    std::string String(const detail::CharacterConfig &cc) {
        std::string s;
        for (boost::optional<char> n; (n = Character(cc));) s.push_back(*n);
        return s;
    }
    Token StringLiteral() {
        auto token = RecordLocation();
        input_.Next();
        std::string data = String(detail::StringConfig());
        auto tail = input_.Lookahead();
        if (not tail or *tail != '"') {
            return token(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kUnclosedStringQuote));
        }
        input_.Next();
        return token(Token::Type::kStringLit, data);
    }

    Token CharLiteral() {
        auto token = RecordLocation();
        input_.Next();
        std::string s = String(detail::CharConfig());
        auto tail = input_.Lookahead();
        if (not tail or *tail != '\'') {
            return token(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kUnclosedCharQuote));
        }
        input_.Next();
        if (s.size() == 1) return token(Token::Type::kCharLit, s.front());
        return token(
            Token::Type::kErrorCode, static_cast<int>(Error::kInvalidChar));
    }
    Token RawToken() {
        auto token = RecordLocation();
        std::string tok = String(detail::RawConfig());
        if (tok.empty())
            return token(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kExpectRawToken));
        return token(Token::Type::kRawString, tok);
    }
    Token PathLiteral() {
        auto token = RecordLocation();
        std::string path;
        if (*input_.Lookahead() == '~') {
            path.push_back('~');
            input_.Next();
        } else {
            path += input_.TakeWhile([](char c) { return c == '.'; });
        }
        auto slash = RecordLocation();
        std::string rest = String(detail::RawConfig());
        if (not rest.empty() and rest.front() != '/') {
            return slash(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kPathExpectSlash));
        }
        path += rest;
        return token(Token::Type::kPathLit, path);
    }
    Token IntLiteral() {
        auto token = RecordLocation();
        auto i = input_.TakeWhile([](char c) { return std::isdigit(c); });
        return token(Token::Type::kIntLit, std::stoi(i));
    }
    Token Identifier() {
        auto loc = input_.NextLocation();
        std::string head(1, *input_.Next());
        std::string rest =
            input_.TakeWhile([](char c) { return isalnum(c) or c == '_'; });
        return Token::FromIdent(
            std::move(head) + std::move(rest), std::move(loc));
    }
    Token UnknownCharacter(char c) {
        return SkipAndMake(Token::Type::kOtherChar, 1, static_cast<int>(c));
    }
    Token NormalLookaheadDispatch() {
        char lookahead = *input_.Lookahead();
        if (lookahead == '"') return StringLiteral();
        if (lookahead == '\'') return CharLiteral();
        if (isdigit(lookahead)) return IntLiteral();
        if (detail::IsIdentHead(lookahead)) return Identifier();
        if (boost::optional<Token> punct = TryPunctuation()) return *punct;
        if (lookahead == '.' or lookahead == '/' or lookahead == '~')
            return PathLiteral();
        return UnknownCharacter(lookahead);
    }
    Token StartInterpolation() {
        auto token = RecordLocation();
        input_.Next();
        auto oc = input_.Lookahead();
        if (not oc)
            return token(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kSingleDollar));
        if (detail::IsIdentHead(*oc)) return Identifier();
        if (*oc == '{') {
            input_.Next();
            return SkipAndMake(Token::Type::kInterStart);
        }
        if (isspace(*oc))
            return token(
                Token::Type::kErrorCode,
                static_cast<int>(Error::kSingleDollar));
        return SkipAndMake(Token::Type::kInvalidChar, 1, int(*oc));
    }
    Token RawLookaheadDispatch() {
        char lookahead = *input_.Lookahead();
        if (lookahead == '$') return StartInterpolation();
        if (lookahead == '"') return StringLiteral();
        if (lookahead == '\'') return CharLiteral();
        if (lookahead == ';') return SkipAndMake(Token::Type::kSemicolon);
        if (lookahead == '.' or lookahead == '/' or lookahead == '~')
            return PathLiteral();
        return RawToken();
    }
    boost::optional<Token> StartOfLine() {
        auto indent = Indentation();
        if (not input_.Lookahead()) return boost::none;
        if (TryLineComment()) {
            LineBreak();
            return Consume();
        }
        return indent;
    }
    boost::optional<Token> NormalMode() {
        if (start_of_line_) return StartOfLine();

        SkipSpaces();
        TryLineComment();
        if (not input_.Lookahead()) return boost::none;
        if (auto lb = TryLineBreak()) return *lb;

        return NormalLookaheadDispatch();
    }
    boost::optional<Token> RawMode() {
        SkipSpaces();

        TryLineComment();
        if (not input_.Lookahead()) return boost::none;
        if (auto lb = TryLineBreak()) return *lb;
        return RawLookaheadDispatch();
    }
    boost::optional<Token> Consume() override {
        if (not input_.Lookahead()) {
            return boost::none;
        }
        return raw_mode_ ? RawMode() : NormalMode();
    }
    detail::SourceStream input_;
    bool start_of_line_ = true;
    bool raw_mode_ = false;
};

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_LEXER_H_
