#ifndef SUSHI_LEXER_LEXER_H_
#define SUSHI_LEXER_LEXER_H_

#include "./context.h"
#include "./detail/lookahead-stream.h"
#include "./token.h"
#include <stack>

namespace sushi {
namespace lexer {

class Lexer : public detail::LookaheadStream<Token> {
  public:
    Lexer(std::istream &is, TokenLocation start)
        : input_(is, start), state_{input_, true} {
        contexts_.push(NormalContext::Factory(state_));
    }

    void NewContext(Context::Factory *f) {
        contexts_.push(f(state_));
    }
    bool DestoryContext() {
        if (contexts_.size() <= 1) return false;
        contexts_.pop();
        return true;
    }
    boost::optional<Token> Consume() override {
        auto result = contexts_.top()->Lex();
        ExecuteAction(result.action);
        if (not result.token and result.action) return Consume();
        return std::move(result.token);
    }

  private:
    void ExecuteAction(boost::optional<Context::Factory *> action) {
        if (not action) return;
        if (*action == nullptr) DestoryContext();
        else NewContext(*action);
    }

    detail::SourceStream input_;
    detail::LexerState state_;
    std::stack<std::unique_ptr<Context>> contexts_;
};

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_LEXER_H_
