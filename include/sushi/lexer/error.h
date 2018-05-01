#ifndef SUSHI_LEXER_ERROR_H_
#define SUSHI_LEXER_ERROR_H_

namespace sushi {
namespace lexer {

enum class Error {
    kUnclosedStringQuote,
    kInvalidChar,
    kUnclosedCharQuote,
    kPathExpectSlash,
    kExpectRawToken,
    kSingleDollar
};

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_ERROR_H_
