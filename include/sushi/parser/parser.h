#ifndef SUSHI_PARSER_PARSER_H_
#define SUSHI_PARSER_PARSER_H_

#include "sushi/ast.h"
#include "sushi/lexer.h"

namespace sushi {
namespace parser {

class Parser {
  public:
    Parser(lexer::Lexer lexer) : lexer_(std::move(lexer)) {}

    ast::Program Parse();

  private:
    lexer::Lexer lexer_;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_PARSER_H_