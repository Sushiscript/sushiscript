#ifndef SUSHI_PARSER_DETAIL_PARSER_STATE_H_
#define SUSHI_PARSER_DETAIL_PARSER_STATE_H_

#include "sushi/lexer/lexer.h"
#include "sushi/parser/detail/lexer-util.h"
#include "sushi/parser/error.h"
#include <functional>

namespace sushi {

namespace parser {

namespace detail {

#define SUSHI_PARSER_ENTER_LOOP(state)                                         \
    sushi::parser::detail::ParserState::LoopGuard sushi_parser_loop_guard(state)

#define SUSHI_PARSER_EXIT_LOOP(state)                                          \
    sushi::parser::detail::ParserState::LoopGuard sushi_parser_loop_guard(     \
        state, true)

#define SUSHI_PARSER_NEW_BLOCK(state, indents)                                 \
    sushi::parser::detail::ParserState::BlockGuard sushi_parser_block_guard(   \
        state, indents)

struct ParserState {
    struct LoopGuard {
        LoopGuard(ParserState &s, bool clear = false) : s(s), clear(clear) {
            if (clear) {
                old = s.loop_level;
                s.loop_level = 0;
            } else {
                ++s.loop_level;
            }
        }
        ~LoopGuard() {
            if (clear) {
                s.loop_level = old;
            } else {
                --s.loop_level;
            }
        }
        ParserState &s;
        bool clear;
        int old = -1;
    };

    struct BlockGuard {
        BlockGuard(ParserState &s, int indents) : s(s) {
            s.indents.push(indents);
        }
        ~BlockGuard() {
            s.indents.pop();
        }
        ParserState &s;
    };

    ParserState(lexer::Lexer l) : lexer(std::move(l)) {}

    int CurrentIndent() const {
        return indents.top();
    }

    int LoopLevel() const {
        return loop_level;
    }

    nullptr_t RecordError(Error::Type t, lexer::Token tok) {
        errors.push_back({t, std::move(tok)});
        return nullptr;
    }

    nullptr_t ExpectToken(lexer::Token::Type t) {
        auto loc = LookaheadAsToken(lexer).location;
        return RecordError(Error::Type::kExpectToken, {t, loc, 0});
    }

    nullptr_t RecordErrorOnLookahead(Error::Type e, bool skip_space = true) {
        auto loc = LookaheadAsToken(lexer, skip_space);
        return RecordError(e, loc);
    }

    boost::optional<lexer::Token>
    AssertLookahead(lexer::Token::Type t, bool skip_space = true) {
        auto tok = Lookahead(lexer, skip_space);
        if (not tok or tok->type != t) {
            ExpectToken(t);
            return boost::none;
        }
        return Next(lexer, skip_space);
    }

    boost::optional<lexer::Token> AssertLookahead(
        std::function<bool(lexer::Token::Type t)> p, Error::Type error_type,
        bool skip_space = true) {
        auto tok = Lookahead(lexer, skip_space);
        if (not tok or not p(tok->type)) {
            auto loc = tok ? tok->location : TokenLocation::Eof();
            RecordError(error_type, tok ? *tok : lexer::Token::Eof());
            return boost::none;
        }
        return Next(lexer, skip_space);
    }

    boost::optional<lexer::Token> LineBreakOr(lexer::Token::Type t) {
        if (auto l = Optional(lexer, t, true)) {
            Optional(lexer, lexer::Token::Type::kLineBreak, false);
            return l;
        }
        return AssertLookahead(lexer::Token::Type::kLineBreak, false);
    }

    lexer::Lexer lexer;
    std::vector<Error> errors;
    std::stack<int> indents;
    int loop_level = 0;
};

} // namespace detail

} // namespace parser

} // namespace sushi

#endif // SUSHI_PARSER_DETAIL_PARSER_STATE_H_