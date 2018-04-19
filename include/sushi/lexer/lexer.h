#ifndef SUSHI_LEXER_LEXER_H_
#define SUSHI_LEXER_LEXER_H_

#include "./lookahead-stream.h"
#include "./token-location.h"
#include "./token.h"
#include "boost/optional.hpp"
#include "sushi/util/optional.h"
#include <cctype>
#include <istream>
#include <string>
#include <utility>

namespace sushi {

namespace detail {

class SourceStream : public LookaheadStream<char> {
  public:
    SourceStream(std::istream &is, TokenLocation start)
        : is_(is), next_loc_(std::move(start)) {}

    boost::optional<char> Next() override {
        auto n = LookaheadStream<char>::Next();
        if (n) {
            if (*n == '\n') {
                next_loc_.NewLine();
            } else {
                next_loc_.NextColumn();
            }
        }
        return n;
    }

    const TokenLocation &NextLocation() const {
        return next_loc_;
    }

  private:
    boost::optional<char> Consume() override {
        if (is_.eof()) {
            return boost::none;
        }
        auto c = is_.get();
        return std::istream::traits_type::not_eof(c)
                   ? boost::make_optional<char>(static_cast<char>(c))
                   : boost::none;
    }

    std::istream &is_;
    TokenLocation next_loc_;
};

} // namespace detail

class Lexer : public LookaheadStream<Token> {
    void Raw(bool b) {
        raw_mode_ = b;
    }
    bool Raw() {
        return raw_mode_;
    }

  private:
    auto RecordLocation() {
        return [loc = input_.NextLocation()](Token::Type t, Token::Data s) {
            return Token{t, std::move(loc), std::move(s)};
        };
    }
    Token SkipAndMake(Token::Type t, int n = 1, Token::Data d = 0) {
        auto location = input_.NextLocation();
        input_.Skip(n);
        return Token{t, std::move(location), std::move(d)};
    }
    boost::optional<Token> Indentation() {
        int num_of_space = 0;
        auto token = RecordLocation();
        for (auto n = input_.Lookahead(); n and *n == ' ';) {
            input_.Next();
        }
        if (LineJoin() or static_cast<bool>(LineBreak())) {
            return Indentation();
        }
        start_of_line_ = false;
        return token(Token::Type::kIndent, num_of_space);
    }
    boost::optional<Token> LineBreak() {
        auto c = input_.Lookahead();
        if (not c or *c != '\n') {
            return boost::none;
        }
        start_of_line_ = true;
        return SkipAndMake(Token::Type::kLineBreak);
    }
    void LineComment() {
        input_.SkipWhile([](char c) { return c != '\n'; });
        input_.Next();
    }
    bool LineJoin() {
        if (input_.LookaheadMany(2) == "\\\n") {
            input_.Take(2);
            return true;
        }
        return false;
    }
    boost::optional<Token> TryDoubleOperator() {
        std::string op = input_.LookaheadMany(2);
        if (op.size() < 2) {
            return boost::none;
        }
        auto iter = Token::DoublePunctuationMap().find(op);
        if (iter == end(Token::DoublePunctuationMap())) {
            return boost::none;
        }
        return SkipAndMake(iter->second, 2, op);
    }
    boost::optional<Token> TrySingleOperator() {
        auto iter = Token::SinglePunctuationMap().find(*input_.Lookahead());
        if (iter == end(Token::SinglePunctuationMap())) {
            return boost::none;
        }
        return SkipAndMake(
            iter->second, 1, std::string(1, *input_.Lookahead()));
    }
    boost::optional<Token> TryPunctuation() {
        using namespace sushi::util::monadic_optional;
        return TryDoubleOperator() | [this]() { return TrySingleOperator(); };
    }
    void SkipSpaces() {
        input_.SkipWhile([](char c) { return isspace(c); });
    }
    Token StringLiteral() {
        auto token = RecordLocation();
        input_.Next();
    }
    Token CharLiteral() {
        auto token = RecordLocation();
        input_.Next();
    }
    Token IntLiteral() {
        auto token = RecordLocation();
        auto i = input_.TakeWhile([](char c) { return std::isdigit(c); });
        return token(Token::Type::kIntLit, std::stoi(i));
    }
    Token Identifier() {
        // todo
    }
    Token UnknownCharacter(char c) {
        return SkipAndMake(Token::Type::kOtherChar, 1, static_cast<int>(c));
    }
    Token NormalLookaheadDispatch() {
        char lookahead = *input_.Lookahead();
        if (lookahead == '"')
            return StringLiteral();
        if (lookahead == '\'')
            return CharLiteral();
        if (isdigit(lookahead))
            return IntLiteral();
        if (isalpha(lookahead) or lookahead == '_')
            return Identifier();
        return TryPunctuation().value_or_eval(
            [lookahead, this]() { UnknownCharacter(lookahead); });
    }
    boost::optional<Token> NormalMode() {
        if (start_of_line_) {
            return Indentation();
        }
        SkipSpaces();
        boost::optional<Token> next_tok;
        return next_tok;
    }
    boost::optional<Token> RawMode() {
        SkipSpaces();
        return boost::none;
    }
    boost::optional<Token> Consume() override {
        if (not input_.Lookahead()) {
            return boost::none;
        }
        return raw_mode_ ? NormalMode() : RawMode();
    }
    detail::SourceStream input_;
    bool start_of_line_ = true;
    bool raw_mode_ = false;
};

} // namespace sushi

#endif // SUSHI_LEXER_LEXER_H_
