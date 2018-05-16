#ifndef SUSHI_TEST_PARSER_UTIL_H_
#define SUSHI_TEST_PARSER_UTIL_H_

#include "sushi/parser/parser.h"
#include "sushi/lexer.h"
#include <sstream>

using namespace sushi;

parser::ParsingResult Parse(std::string s) {
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

#endif // SUSHI_TEST_PARSER_UTIL_H_