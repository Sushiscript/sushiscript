#ifndef SUSHI_TEST_PARSER_UTIL_H_
#define SUSHI_TEST_PARSER_UTIL_H_

#include "boost/algorithm/string/join.hpp"
#include "sushi/ast/to-string.h"
#include "sushi/lexer.h"
#include "sushi/parser/parser.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace sushi;

inline parser::ParsingResult Parse(std::string s) {
    std::istringstream iss(s);
    lexer::Lexer lexer(iss, {"", 1, 1});
    parser::Parser p(std::move(lexer));
    return p.Parse();
}

#define TDL(type, data, line, col)                                             \
    lexer::Token {                                                             \
        lexer::Token::Type::type, {"", line, col}, data                        \
    }

#define TD(type, data) TDL(type, data, 1, 1)
#define TK(type) TD(type, 0)

namespace std {

inline std::ostream &
operator<<(std::ostream &os, std::vector<parser::Error> es) {
    std::vector<std::string> ess;
    std::transform(
        begin(es), end(es), std::back_inserter(ess),
        [](const parser::Error &e) { return e.ToString(); });
    os << boost::join(ess, "\n") << '\n';
    return os;
}

} // namespace std

inline void ParseSuccess(const std::string &source, const std::string &expect) {
    SCOPED_TRACE(source);
    auto result = Parse(source);
    EXPECT_EQ(sushi::ast::ToString(result.program, 0), expect);
    EXPECT_TRUE(result.errors.empty()) << result.errors;
}

#endif // SUSHI_TEST_PARSER_UTIL_H_