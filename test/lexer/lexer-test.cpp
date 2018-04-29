#include "sushi/lexer/lexer.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace sushi;

lexer::Lexer FromStream(std::istream &is) {
    return lexer::Lexer(is, TokenLocation{"", 1, 1});
}

using Tokens = std::vector<lexer::Token>;

Tokens CollectAll(lexer::Lexer &l) {
    std::vector<lexer::Token> toks;
    for (boost::optional<lexer::Token> ot; (ot = l.Next());) {
        toks.push_back(*ot);
    }
    return toks;
}

bool AllWeakEqual(const Tokens &t1, const Tokens &t2) {
    if (t1.size() != t2.size()) return false;
    for (int i = 0; i < t1.size(); ++i) {
        if (not lexer::Token::WeakEqual(t1[i], t2[i])) return false;
    }
    return true;
}

Tokens FromString(const std::string &s) {
    std::istringstream iss(s);
    auto lex = FromStream(iss);
    return CollectAll(lex);
}

#define TK(type, data)                                                         \
    lexer::Token {                                                             \
        lexer::Token::Type::type, {"", 1, 1}, data                             \
    }

namespace std {

std::ostream &operator<<(std::ostream &os, const Tokens &ts) {
    if (ts.empty()) {
        os << "[]";
        return os;
    }
    os << '[';
    for (int i = 0; i < ts.size() - 1; ++i) {
        os << ts[i] << ' ';
    }
    os << ts.back() << ']';
    return os;
}

} // namespace std

TEST(SingleTokenTest, TestIdentifier) {
    Tokens t1{TK(kIdent, "i")};
    EXPECT_PRED2(AllWeakEqual, FromString("i"), t1);
}
