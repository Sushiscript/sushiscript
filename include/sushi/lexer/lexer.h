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
    boost::optional<Token> DoubleOperator() {
        auto op = input_.LookaheadMany(2);
        if (op.size() < 2) {
            return boost::none;
        }
        if (op == ">=")
            return SkipAndMake(Token::Type::kGreaterEq, 2);
        if (op == "<=")
            return SkipAndMake(Token::Type::kLessEq, 2);
        if (op == "==")
            return SkipAndMake(Token::Type::kDoubleEq, 2);
        if (op == "!=")
            return SkipAndMake(Token::Type::kNotEqual, 2);
        return boost::none;
    }
    boost::optional<Token> SingleOperator() {
        switch (*input_.Lookahead()) {
        case '+':
            return SkipAndMake(Token::Type::kPlus);
        case '-':
            return SkipAndMake(Token::Type::kMinus);
        case '*':
            return SkipAndMake(Token::Type::kStar);
        case '%':
            return SkipAndMake(Token::Type::kPercent);
        case '<':
            return SkipAndMake(Token::Type::kLAngle);
        case '>':
            return SkipAndMake(Token::Type::kRAngle);
        case ',':
            return SkipAndMake(Token::Type::kComma);
        case ':':
            return SkipAndMake(Token::Type::kColon);
        case '!':
            return SkipAndMake(Token::Type::kExclamation);
        case '$':
            return SkipAndMake(Token::Type::kDollar);
        default:
            return boost::none;
        }
    }
    boost::optional<Token> Integer() {
        auto token = RecordLocation();
        auto i = input_.TakeWhile([](char c) { return std::isdigit(c); });
        return i.empty() ? boost::optional<Token>()
                         : token(Token::Type::kIntLit, std::stoi(i));
    }
    boost::optional<Token> Punctuation() {
        using namespace sushi::util::monadic_optional;
        return DoubleOperator() | [this]() { return SingleOperator(); };
    }
    boost::optional<Token> Identifier() {
        return boost::none;
    }
    Token InvalidCharacterError() {
        auto c = input_.Lookahead();
        // return Token();
    }
    boost::optional<Token> Consume() override {
        if (start_of_line_) {
            return Indentation();
        }
        input_.SkipWhile([](char c) { return isspace(c); });
        boost::optional<Token> next_tok;
        return next_tok;
    }
    detail::SourceStream input_;
    bool start_of_line_ = true;
};

} // namespace sushi

#endif // SUSHI_LEXER_LEXER_H_
