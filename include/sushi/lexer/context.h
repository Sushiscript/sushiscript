#ifndef SUSHI_LEXER_CONTEXT_H_
#define SUSHI_LEXER_CONTEXT_H_

#include "./detail/character-config.h"
#include "./detail/lexeme.h"
#include "./detail/lexer-state.h"
#include "./token.h"
#include "boost/optional.hpp"
#include "sushi/util/meta.h"
#include <memory>
#include <utility>

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
    static LexResult Enter() {
        static_assert(
            std::is_base_of<Context, C>::value, "Must Enter to a context");
        return LexResult(boost::none, C::Factory);
    }

    template <typename C>
    static LexResult SkipEnter(detail::LexerState &s, int n = 1) {
        s.input.Skip(n);
        return Enter<C>();
    }

    static LexResult EmitExit(Token token) {
        return LexResult(std::move(token), nullptr);
    }

    template <typename C>
    static LexResult EmitEnter(Token token) {
        return LexResult(std::move(token), C::Factory);
    }

    template <typename C>
    static LexResult
    SkipEmitEnter(detail::LexerState &s, Token token, int n = 1) {
        s.input.Skip(n);
        return EmitEnter<C>(token);
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

enum class SegmentReason { kFalseStop, kRestrict, kInterpolation, kEof };

template <typename CharCfg>
struct InterpolateConfig {
    using CharacterConfig = CharCfg;

    using Super = InterpolateConfig<CharacterConfig>;

    Context::LexResult
    Segment(std::string data, TokenLocation l, SegmentReason) {
        return Token{Token::Type::kSegment, std::move(l), std::move(data)};
    }

    Context::LexResult Finish(LexerState &s, TokenLocation l) {
        return Context::EmitExit({Token::Type::kInterDone, std::move(l), 0});
    }
};

struct StrInterCfg : InterpolateConfig<StringConfig> {
    Context::LexResult Finish(LexerState &s, TokenLocation l) {
        auto tail = s.input.Lookahead();
        if (not tail or *tail != '"') {
            return Context::EmitExit(
                Token::Error(std::move(l), Error::kUnclosedStringQuote));
        }
        s.input.Next();
        return Super::Finish(s, std::move(l));
    }
};

using RawInterCfg = InterpolateConfig<RawConfig>;

struct PathInterCfg : InterpolateConfig<RawConfig> {
    Context::LexResult
    Segment(std::string data, TokenLocation l, SegmentReason r) {
        if (not start) return Super::Segment(std::move(data), std::move(l), r);
        start = false;
        int offset = ExpectSlash(data);
        if ((offset >= data.size() and (r == SegmentReason::kFalseStop or
                                        r == SegmentReason::kInterpolation)) or
            (offset < data.size() and data[offset] != '/')) {
            TokenLocation errl{l.src_path, l.line, l.column + offset};
            return Token::Error(std::move(errl), Error::kPathExpectSlash);
        }
        return Super::Segment(std::move(data), std::move(l), r);
    }

  private:
    int ExpectSlash(std::string data) {
        if (data.front() == '~') return 1;
        auto pos = data.find_first_not_of('.');
        if (pos == std::string::npos) return data.size();
        return pos;
    }
    bool start = true;
};

template <typename InterConfig>
class InterpolateContext : public Context {
  public:
    InterpolateContext(detail::LexerState &state)
        : Context(state), cc(MakeConfig()){};

    LEX_CONTEXT_FACTORY(InterpolateContext<InterConfig>)

    static std::unique_ptr<CharacterConfig> MakeConfig() {
        std::string new_list{1, '$'};
        return std::make_unique<CustomConfig>(
            std::make_unique<typename InterConfig::CharacterConfig>(),
            new_list);
    }

    Context::LexResult Lex() override {
        auto &input = state.input;
        auto l = input.NextLocation();
        auto p = ExtractSegment();
        std::string data = p.first;
        SegmentReason reason = p.second;
        if (not data.empty())
            return ic.Segment(std::move(data), std::move(l), reason);
        if (reason == SegmentReason::kInterpolation)
            return Context::EmitEnter<NormalContext>(
                SkipAndMake(state, Token::Type::kInterStart, 2));
        // reason == kRestrict or reason == kEof
        return ic.Finish(state, l);
    }

  private:
    std::pair<std::string, SegmentReason> ExtractSegment() {
        std::string data;
        SegmentReason reason = SegmentReason::kFalseStop;
        for (;;) {
            data += detail::String(state, *cc);
            reason = CheckStopReason();
            if (reason != SegmentReason::kFalseStop) break;
            ConsumeSingleDollar(data);
        }
        return {data, reason};
    }

    void ConsumeSingleDollar(std::string &s) {
        auto oc = state.input.Lookahead();
        if (oc and *oc == '$') s.push_back(*state.input.Next());
    }

    SegmentReason CheckStopReason() {
        auto oc = state.input.Lookahead();
        if (not oc) return SegmentReason::kEof;
        if (*oc != '$') return SegmentReason::kRestrict;
        if (state.input.LookaheadMany(2) == "${")
            return SegmentReason::kInterpolation;
        return SegmentReason::kFalseStop;
    }

    std::unique_ptr<CharacterConfig> cc;
    InterConfig ic;
};

} // namespace detail

using StringLitContext =
    detail::InterpolateContext<typename detail::StrInterCfg>;
using PathLitContext =
    detail::InterpolateContext<typename detail::PathInterCfg>;
using RawTokenContext =
    detail::InterpolateContext<typename detail::RawInterCfg>;

#undef LEX_CONTEXT
#undef LEX_CONTEXT_FACTORY

} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_CONTEXT_H_