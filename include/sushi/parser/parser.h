#ifndef SUSHI_PARSER_PARSER_H_
#define SUSHI_PARSER_PARSER_H_

#include "sushi/ast.h"
#include "sushi/lexer.h"
#include "sushi/parser/error.h"

#include <memory>
#include <stack>
#include <vector>

namespace sushi {
namespace parser {

struct ParsingResult {
    ast::Program program;
    std::vector<Error> errors;
};

class Parser {
  public:
    Parser(lexer::Lexer lexer) : lexer_(std::move(lexer)) {
        indents_.push(0);
    }

    ParsingResult Parse() {
        auto p = Program();
        return {std::move(p), std::move(current_errors_)};
    }

  private:
    ast::Program Program();

    void DiscoverIndent();

    std::unique_ptr<ast::Statement> Line();

    std::unique_ptr<ast::Statement> Statement();

    std::unique_ptr<ast::Expression> Expression();

    std::unique_ptr<ast::Expression> AtomExpression();

    nullptr_t EncounterError(Error::Type t, lexer::Token pos) {
        current_errors_.emplace_back(t, std::move(pos));
        return nullptr;
    }

    int CurrentIndent() const {
        return indents_.top();
    }

    lexer::Lexer lexer_;
    std::vector<Error> current_errors_;
    std::stack<int> indents_;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_PARSER_H_