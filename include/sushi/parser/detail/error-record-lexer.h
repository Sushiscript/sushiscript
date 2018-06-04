#ifndef SUSHI_PARSER_DETAIL_ERROR_RECORD_LEXER_H_
#define SUSHI_PARSER_DETAIL_ERROR_RECORD_LEXER_H_

#include "./lexer-util.h"
#include "sushi/lexer/lexer.h"
#include <functional>

namespace sushi {
namespace parser {
namespace detail {

class ErrorRecordLexer : public lexer::Lexer {
  public:
    using ErrorCallback = std::function<void(lexer::Token e)>;

    ErrorRecordLexer(std::istream &is, TokenLocation start, ErrorCallback cb)
        : lexer::Lexer(is, std::move(start)), on_error_(cb) {}

    ErrorRecordLexer(lexer::Lexer lexer, ErrorCallback cb)
        : lexer::Lexer(std::move(lexer)), on_error_(cb) {}

    boost::optional<lexer::Token> Consume() override {
        boost::optional<lexer::Token> result;
        for (; (result = lexer::Lexer::Consume()) and IsError(result->type);) {
            on_error_(std::move(*result));
        }
        return result;
    }

  private:
    ErrorCallback on_error_;
};

} // namespace detail
} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_DETAIL_ERROR_RECORD_LEXER_H_