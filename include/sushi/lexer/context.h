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

    struct LexResult {
        LexResult() = default;
        LexResult(boost::optional<Token> token, Factory *action)
            : token(std::move(token)), action(action) {}

        LexResult(Token token) : token(token) {}
        LexResult(boost::optional<Token> token) : token(std::move(token)) {}
        LexResult(boost::none_t none) {}

        boost::optional<Token> token;
        boost::optional<Factory *> action;
    };

    template <typename C>
    static LexResult Transfer() {
        static_assert(
            std::is_base_of<Context, C>::value, "Must transfer to a context");
        return LexResult(boost::none, C::Factory);
    }

    template <typename C>
    static LexResult SkipTransfer(detail::LexerState &s, int n = 1) {
        s.input.Skip(n);
        return Transfer<C>();
    }

    static LexResult EmitAndExit(Token token) {
        return LexResult(token, nullptr);
    }

    Context(detail::LexerState &state) : state(state) {}
    virtual LexResult Lex() = 0;
    virtual ~Context() = default;

    detail::LexerState &state;
};

#define LEX_CONTEXT(ContextName)                                               \
  public:                                                                      \
    ContextName(detail::LexerState &state) : Context(state) {}                 \
    static std::unique_ptr<Context> Factory(detail::LexerState &state) {       \
        return std::make_unique<ContextName>(state);                           \
    }                                                                          \
    Context::LexResult Lex() override;

class NormalContext : public Context {
    LEX_CONTEXT(NormalContext)

  private:
    Context::LexResult StartOfLine();
};

class RawContext : public Context {
    LEX_CONTEXT(RawContext)
};

class StringLitContext : public Context {
    LEX_CONTEXT(StringLitContext)
};

class RawTokenContext : public Context {
    LEX_CONTEXT(RawTokenContext)
};

class PathLitContext : public Context {
    LEX_CONTEXT(PathLitContext)
};

#undef LEX_CONTEXT

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_CONTEXT_H_