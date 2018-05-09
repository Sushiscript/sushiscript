#include "sushi/parser/parser.h"
#include "boost/optional.hpp"
#include "boost/variant.hpp"
#include "sushi/parser/detail/lexer-util.h"

using boost::get;
using boost::optional;
using std::unique_ptr;

namespace sushi {

using lexer::Token;

namespace parser {

using namespace detail;

bool Parser::AssertLookahead(lexer::Token::Type t, bool skip_space) {
    auto tok = Lookahead(lexer_, skip_space);
    if (not tok or tok->type != t) {
        auto loc = tok ? tok->location : TokenLocation::Eof();
        RecordError(Error::Type::kExpectToken, {t, std::move(loc), 0});
        return false;
    }
    return true;
}

bool Parser::AssertNext(lexer::Token::Type t, bool skip_space) {
    auto result = AssertLookahead(t, skip_space);
    Next(lexer_, skip_space);
    return result;
}

ast::Program Parser::Program() {
    int indent = DetermineBlockIndent();
    if (indent <= CurrentIndent()) {
        return {};
    }
    return WithBlock(indent, &Parser::Block);
}

ast::Program Parser::Block() {
    ast::Program p;
    for (;;) {
        auto ostmt = CurrentBlockStatement();
        if (not ostmt)
            break;
        if (*ostmt != nullptr)
            p.statements.push_back(std::move(*ostmt));
    }
    return p;
}
optional<unique_ptr<ast::Statement>> Parser::CurrentBlockStatement() {
    auto lookahead = lexer_.Lookahead();
    if (not lookahead)
        return boost::none;
    if (lookahead->type != Token::Type::kIndent)
        return Statement();

    // type != Type::kIndent
    int indent = lookahead->IntData();
    if (indent < CurrentIndent())
        return boost::none;
    if (indent > CurrentIndent()) {
        RecordError(Error::Type::kUnexpectIndent, lexer_.Next());
        WithBlock(indent, &Parser::Statement);
        return nullptr;
    }
    lexer_.Next();
    return Statement();
}

int Parser::DetermineBlockIndent() {
    auto lookahead = lexer_.Lookahead();
    if (not lookahead) {
        return -1;
    }
    if (lookahead->type == Token::Type::kIndent) {
        return lookahead->IntData();
    }
    return lookahead->location.column;
}

unique_ptr<ast::Statement> Parser::Statement() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::Expression() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::AtomExpression() {
    return nullptr;
}

} // namespace parser
} // namespace sushi