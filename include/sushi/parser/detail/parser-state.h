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
    sushi::parser::detail::ParserState::LoopGuard sushi_parser_loop_guard(     \
        state, true)

#define SUSHI_PARSER_EXIT_LOOP(state)                                          \
    sushi::parser::detail::ParserState::LoopGuard sushi_parser_loop_guard(     \
        state, false)

#define SUSHI_PARSER_NEW_BLOCK(state, indents)                                 \
    sushi::parser::detail::ParserState::BlockGuard sushi_parser_block_guard(   \
        state, indents)

struct ParserState {
    struct LoopGuard {
        LoopGuard(ParserState &s, bool now_in_loop) : s(s), old(s.inside_loop) {
            s.inside_loop = now_in_loop;
        }
        ~LoopGuard() {
            s.inside_loop = old;
        }
        ParserState &s;
        bool old;
    };

    struct BlockGuard {
        BlockGuard(ParserState &s, bool indents) : s(s) {
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

    nullptr_t RecordError(Error::Type t, lexer::Token tok) {
        errors.push_back({t, std::move(tok)});
        return nullptr;
    }

    boost::optional<lexer::Token>
    AssertLookahead(lexer::Token::Type t, bool skip_space = true) {
        auto tok = Lookahead(lexer, skip_space);
        if (not tok or tok->type != t) {
            auto loc = tok ? tok->location : TokenLocation::Eof();
            RecordError(Error::Type::kExpectToken, {t, loc, 0});
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
    bool inside_loop = false;
};

} // namespace detail

} // namespace parser

} // namespace sushi

#endif // SUSHI_PARSER_DETAIL_PARSER_STATE_H_