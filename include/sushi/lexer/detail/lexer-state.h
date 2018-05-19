#ifndef SUSHI_LEXER_DETAIL_LEXER_STATE_H_
#define SUSHI_LEXER_DETAIL_LEXER_STATE_H_

#include "./source-stream.h"

namespace sushi {
namespace lexer {
namespace detail {

struct LexerState {
    SourceStream &input;
    bool line_start;
};

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_LEXER_STATE_H_