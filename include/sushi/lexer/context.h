#ifndef SUSHI_LEXER_CONTEXT_H_
#define SUSHI_LEXER_CONTEXT_H_

#include "./detail/character-config.h"
#include "./detail/lexeme.h"
#include "./detail/lexer-state.h"
#include "./token.h"
#include "sushi/util/meta.h"
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
        // none    -- no action
        // nullptr -- pop context
        // other   -- push context
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

#define LEX_CONTEXT_FACTORY(ContextName)                                       \
    static std::unique_ptr<Context> Factory(detail::LexerState &state) {       \
        return std::make_unique<ContextName>(state);                           \
    }

#define LEX_CONTEXT(ContextName)                                               \
  public:                                                                      \
    ContextName(detail::LexerState &state) : Context(state) {}                 \
    Context::LexResult Lex() override;                                         \
    LEX_CONTEXT_FACTORY(ContextName)

class NormalContext : public Context {
    LEX_CONTEXT(NormalContext)

  private:
    Context::LexResult StartOfLine();
};

class RawContext : public Context {
    LEX_CONTEXT(RawContext)
};

namespace detail {

template <typename CharCfg, Token::Type Seg, Token::Type Done>
struct InterpolateConfig {
    using CharacterConfig = CharCfg;

    static constexpr Token::Type kDone = Done;
    static constexpr Token::Type kSeg = Seg;

    using Super = InterpolateConfig<CharacterConfig, kSeg, kDone>;

    Context::LexResult Segment(std::string data, TokenLocation l) {
        return Context::LexResult(
            Token{Seg, std::move(l), std::move(data)}, NormalContext::Factory);
    }

    Context::LexResult
    Finish(LexerState &s, std::string data, TokenLocation l) {
        return Context::EmitAndExit({Done, std::move(l), std::move(data)});
    }
};

class StrInterCfg
    : InterpolateConfig<
          StringConfig, Token::Type::kStringLitSeg, Token::Type::kStringLit> {

    Context::LexResult
    Finish(LexerState &s, std::string data, TokenLocation l) {
        auto tail = s.input.Lookahead();
        if (not tail or *tail != '"') {
            return Context::EmitAndExit(
                Token::Error(std::move(l), Error::kUnclosedStringQuote));
        }
        s.input.Next();
        return Super::Finish(s, std::move(data), std::move(l));
    }
};

using RawInterCfg = InterpolateConfig<
    RawConfig, Token::Type::kRawStringSeg, Token::Type::kRawString>;

struct PathInterCfg
    : InterpolateConfig<
          RawConfig, Token::Type::kPathLitSeg, Token::Type::kPathLit> {

    Context::LexResult Segment(std::string data, TokenLocation l) {
        if (start) {
            start = false;
            if (auto slash_p = MissingSlash(data)) {
                return {MissingSlashError(*slash_p, std::move(l)),
                        NormalContext::Factory};
            }
        }
        return Super::Segment(std::move(data), std::move(l));
    }

    Context::LexResult
    Finish(LexerState &s, std::string data, TokenLocation l) {
        if (start) {
            if (auto slash_p = MissingSlash(data)) {
                return {MissingSlashError(*slash_p, std::move(l)), nullptr};
            }
        }
        return Super::Finish(s, std::move(data), std::move(l));
    }

  private:
    boost::optional<int> MissingSlash(std::string data) {
        if (data.front() == '~') {
            if (data.size() > 1 and data[1] != '/') return 1;
            return boost::none;
        }
        auto pos = data.find_first_not_of('.');
        if (pos != std::string::npos and data[pos] != '/') return pos;
        return boost::none;
    }
    Token MissingSlashError(int p, TokenLocation l) {
        return Token::Error(
            {l.src_path, l.line, l.column + p},
            static_cast<int>(Error::kPathExpectSlash));
    }
    bool start = true;
};

template <typename InterConfig>
class InterpolateContext : public Context {
  public:
    static constexpr char kInterStartSeq[] = "${";

    InterpolateContext(detail::LexerState &state)
        : Context(state), cc(MakeConfig()){};

    LEX_CONTEXT_FACTORY(InterpolateContext<InterConfig>)

    static std::unique_ptr<CharacterConfig> MakeConfig() {
        std::string new_list{1, kInterStartSeq[0]};
        return std::make_unique<CustomConfig>(
            std::make_unique<InterConfig::CharacterConfig>(), new_list);
    }

    Context::LexResult Lex() override {
        auto &input = state.input;
        auto l = input.NextLocation();
        size_t inter_start_len = sushi::util::ArrayLength(kInterStartSeq);

        std::string data;
        for (;;) {
            data += detail::String(state, cc);
            auto n1 = input.Lookahead();
            if (n1 != kInterStartSeq[0]) // restricted by cc
                return ic.Finish(state, std::move(data), std::move(l));
            auto actual = input.LookaheadMany(inter_start_len);
            if (actual == kInterStartSeq) {
                input.Skip(inter_start_len);
                return ic.Segment(std::move(data), std::move(l));
            }
            data += *input.Next(); // skip the false interpolation head
        }
    }

  private:
    std::unique_ptr<CharacterConfig> cc;
    InterConfig ic;
};

} // namespace detail

using StringLitContext = detail::InterpolateContext<detail::StrInterCfg>;
using PathLitContext = detail::InterpolateContext<detail::PathInterCfg>;
using RawTokenContext = detail::InterpolateContext<detail::RawInterCfg>;

#undef LEX_CONTEXT
#undef LEX_CONTEXT_FACTORY

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_CONTEXT_H_