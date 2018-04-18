#ifndef SUSHI_LEXER_LEXER_H_
#define SUSHI_LEXER_LEXER_H_

#include "./lookahead-stream.h"
#include "./token-location.h"
#include "./token.h"
#include "boost/optional.hpp"
#include <istream>
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

  private:
    virtual boost::optional<char> Consume() override {
        if (is_.eof()) {
            return boost::none;
        }
        auto c = is_.get();
        return std::istream::traits_type::not_eof(c)
                   ? boost::make_optional<char>(c)
                   : boost::none;
    }

    std::istream &is_;
    TokenLocation next_loc_;
};

} // namespace detail

class Lexer : public LookaheadStream<Token> {
  public:
  private:
    boost::optional<Token> Consume() override {
        return boost::none;
    }
    detail::SourceStream input_;
};

} // namespace sushi

#endif // SUSHI_LEXER_LEXER_H_
