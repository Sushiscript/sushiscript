#ifndef SUSHI_LEXER_DETAIL_SOURCE_STREAM_H_
#define SUSHI_LEXER_DETAIL_SOURCE_STREAM_H_

#include "./lookahead-stream.h"
#include "sushi/lexer/token-location.h"
#include <istream>

namespace sushi {
namespace lexer {
namespace detail {

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
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_SOURCE_STREAM_H_