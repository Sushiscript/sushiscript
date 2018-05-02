#ifndef SUSHI_LEXER_CONTEXT_H_
#define SUSHI_LEXER_CONTEXT_H_

#include "./detail/lexer-state.h"
#include "./token.h"
#include "boost/optional.hpp"
#include <memory>

namespace sushi {
namespace lexer {

class Context {
  public:
    using Pointer = std::unique_ptr<Context>;
    using Factory = Pointer(detail::LexerState &);

    Context(detail::LexerState &state) : state(state) {}
    virtual boost::optional<Token> Lex() = 0;
    virtual ~Context() = default;

    detail::LexerState &state;
};

class NormalContext : public Context {
  public:
    NormalContext(detail::LexerState &state) : Context(state) {}

    static std::unique_ptr<Context> Factory(detail::LexerState &state) {
        return std::make_unique<NormalContext>(state);
    }

    boost::optional<Token> Lex() override;
};

class RawContext : public Context {
  public:
    RawContext(detail::LexerState &state) : Context(state) {}

    static std::unique_ptr<Context> Factory(detail::LexerState &state) {
        return std::make_unique<RawContext>(state);
    }

    boost::optional<Token> Lex() override;
};

class StringLiteralContext : public Context {
  public:
    StringLiteralContext(detail::LexerState &state) : Context(state) {}

    static std::unique_ptr<Context> Factory(detail::LexerState &state) {
        return std::make_unique<StringLiteralContext>(state);
    }

    boost::optional<Token> Lex() override;
};

class RawLiteralContext : public Context {
  public:
    RawLiteralContext(detail::LexerState &state) : Context(state) {}
    static std::unique_ptr<Context> Factory(detail::LexerState &state) {
        return std::make_unique<RawLiteralContext>(state);
    }

    boost::optional<Token> Lex() override;
};

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_CONTEXT_H_