#ifndef SUSHI_TEST_LEXER_UTIL_H_
#define SUSHI_TEST_LEXER_UTIL_H_

#include "sushi/lexer/detail/character-config.h"
#include "sushi/lexer/error.h"
#include "sushi/lexer/lexer.h"
#include "sushi/lexer/token.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace sushi;

namespace sushi {
namespace lexer {
namespace test {

inline lexer::Lexer FromStream(std::istream &is) {
    return lexer::Lexer(is, TokenLocation{"", 1, 1});
}

using Tokens = std::vector<lexer::Token>;

inline Tokens CollectAll(lexer::Lexer &l) {
    std::vector<lexer::Token> toks;
    for (boost::optional<lexer::Token> ot; (ot = l.Next());) {
        toks.push_back(*ot);
    }
    return toks;
}

template <bool Strong = false>
bool AllEqual(const Tokens &t1, const Tokens &t2) {
    if (t1.size() != t2.size()) return false;
    for (int i = 0; i < t1.size(); ++i) {
        bool eq = Strong ? t1[i] == t2[i] : Token::WeakEqual(t1[i], t2[i]);
        if (not eq) return false;
    }
    return true;
}

inline Tokens FromString(const std::string &s, bool raw_mode = false) {
    std::istringstream iss(s);
    auto lex = FromStream(iss);
    if (raw_mode) {
        lex.NewContext(RawContext::Factory);
    }
    return CollectAll(lex);
}

#define TDL(type, data, line, col)                                             \
    lexer::Token {                                                             \
        lexer::Token::Type::type, {"", line, col}, data                        \
    }

#define TD(type, data) TDL(type, data, 1, 1)
#define TK(type) TD(type, 0)
#define TL(type, line, col) TDL(type, 0, line, col)
#define TEL(err, line, col)                                                    \
    TDL(kErrorCode, static_cast<int>(sushi::lexer::Error::err), line, col)
#define TE(err) TEL(err, 1, 1);

template <bool Raw = false, bool Strong = false, typename... Args>
void StringIsTokens(const std::string &s, Args... tokens) {
    SCOPED_TRACE(s);
    Tokens expected{tokens...};
    Tokens test = FromString(s, Raw);
    EXPECT_PRED2(AllEqual<Strong>, test, expected);
}
template <bool Raw = false, bool Strong = false>
void StringIsTokens(const std::string &s, const Tokens &expected) {
    Tokens test = FromString(s, Raw);
    EXPECT_PRED2(AllEqual<Strong>, test, expected);
}

template <bool Raw = false, typename... Args>
void ExactStrIsToks(const std::string &s, Args... tokens) {
    StringIsTokens<Raw, true>(s, tokens...);
}

template <bool Raw = false>
void ExactStrIsToks(const std::string &s, const Tokens &toks) {
    StringIsTokens<Raw, true>(s, toks);
}

template <bool Strong = false, typename... Args>
void NoIndentStrIsToks(const std::string &s, Args... tokens) {
    StringIsTokens<false, Strong>(s, TK(kIndent), tokens...);
}

template <bool Strong = false>
void NoIndentStrIsToks(const std::string &s, const Tokens &tokens) {
    StringIsTokens<false, Strong>(s, TK(kIndent), tokens);
}

template <bool Strong = false, typename... Args>
void RawStrIsTokens(const std::string &s, Args... tokens) {
    StringIsTokens<true, Strong>(s, tokens...);
}
template <bool Strong = false>
void RawStrIsTokens(const std::string &s, const Tokens &tokens) {
    StringIsTokens<true, Strong>(s, tokens);
}

} // namespace test
} // namespace lexer
} // namespace sushi

namespace std {

inline std::ostream &
operator<<(std::ostream &os, const sushi::lexer::test::Tokens &ts) {
    if (ts.empty()) {
        os << "[]";
        return os;
    }
    os << '[';
    for (int i = 0; i < ts.size() - 1; ++i) {
        os << ts[i] << ", ";
    }
    os << ts.back() << ']';
    return os;
}

} // namespace std

#endif