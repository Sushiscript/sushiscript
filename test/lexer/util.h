#ifndef SUSHI_TEST_LEXER_UTIL_H_
#define SUSHI_TEST_LEXER_UTIL_H_

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

inline bool AllWeakEqual(const Tokens &t1, const Tokens &t2) {
    if (t1.size() != t2.size()) return false;
    for (int i = 0; i < t1.size(); ++i) {
        if (not lexer::Token::WeakEqual(t1[i], t2[i])) return false;
    }
    return true;
}

inline Tokens FromString(const std::string &s, bool raw_mode = false) {
    std::istringstream iss(s);
    auto lex = FromStream(iss);
    lex.Raw(raw_mode);
    return CollectAll(lex);
}

#define TK(type, data)                                                         \
    lexer::Token {                                                             \
        lexer::Token::Type::type, {"", 1, 1}, data                             \
    }

template <bool Raw = false, typename... Args>
void StringIsTokens(const std::string &s, Args... tokens) {
    Tokens expected{tokens...};
    Tokens test = FromString(s, Raw);
    EXPECT_PRED2(AllWeakEqual, test, expected);
}

template <typename... Args>
void NoIndentStrIsToks(const std::string &s, Args... tokens) {
    StringIsTokens(s, TK(kIndent, 0), tokens...);
}

template <typename... Args>
void RawStrIsTokens(const std::string &s, Args... tokens) {
    StringIsTokens<true>(s, tokens...);
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