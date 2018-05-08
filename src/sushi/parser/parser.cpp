#include "sushi/parser/parser.h"
#include "boost/optional.hpp"
#include "boost/variant.hpp"

using boost::get;
using boost::optional;
using lexer::Token;

namespace sushi {
namespace parser {

ast::Program Parser::Program() {
    DiscoverIndent();
    std::vector<std::unique_ptr<ast::Statement>> stmts;
    for (optional<Token> tok; (tok = lexer_.Lookahead());) {
        auto stmt = Line();
        if (not stmt) continue;
        stmts.push_back(std::move(stmt));
    }
    indents_.pop();
    return stmts;
}

void Parser::DiscoverIndent() {
    auto lookahead = lexer_.Lookahead();
    if (lookahead and lookahead->type == Token::Type::kIndent) {
        indents_.push(lookahead->content);
    } else {
        indents_.push(CurrentIndent());
    }
}

std::unique_ptr<ast::Statement> Parser::Line() {
    auto lookahead = lexer_.Lookahead();
    if (lookahead and lookahead->type == Token::Type::kIndent) {
        auto indent = *lexer_.Next();
        if (indent.IntData() != CurrentIndent()) {
            Statement();
            return EncounterError(
                Error::Type::kUnexpectIndent, std::move(indent));
        }
    }
    return Statement();
}

std::unique_ptr<ast::Statement> Parser::Statement() {}

std::unique_ptr<ast::Expression> Parser::Expression() {}

std::unique_ptr<ast::Expression> Parser::AtomExpression() {}

} // namespace parser
} // namespace sushi