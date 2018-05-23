#ifndef SUSHI_LEXER_DETAIL_LEXER_STATE_H_
#define SUSHI_LEXER_DETAIL_LEXER_STATE_H_

#include "./source-stream.h"

namespace sushi {
namespace lexer {
namespace detail {

struct LexerState {
    LexerState(SourceStream input, bool line_start)
        : input(std::move(input)), line_start(line_start) {}

    SourceStream input;

    bool line_start;

    void LineStart(bool b) {
        line_start = b;
    }
    bool LineStart() const {
        return line_start;
    }
};

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_LEXER_STATE_H_