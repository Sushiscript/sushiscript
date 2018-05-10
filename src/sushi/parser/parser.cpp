#include "sushi/parser/parser.h"
#include "boost/optional.hpp"
#include "boost/variant.hpp"
#include "sushi/parser/detail/lexer-util.h"

using std::unique_ptr;
using boost::get;
using boost::optional;

namespace sushi {

using lexer::Token;

namespace parser {

using namespace detail;

ast::Program Parser::Program() {
    int indent = DetermineBlockIndent();
    if (indent <= s_.CurrentIndent()) {
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
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead)
        return boost::none;
    if (lookahead->type != Token::Type::kIndent)
        return Statement();

    // type != Type::kIndent
    int indent = lookahead->IntData();
    if (indent < s_.CurrentIndent())
        return boost::none;
    if (indent > s_.CurrentIndent()) {
        s_.RecordError(Error::Type::kUnexpectIndent, *Next(s_.lexer, false));
        WithBlock(indent, &Parser::Statement);
        return boost::make_optional<unique_ptr<ast::Statement>>(nullptr);
    }
    Next(s_.lexer, false);
    return Statement();
}

int Parser::DetermineBlockIndent() {
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead) {
        return -1;
    }
    if (lookahead->type == Token::Type::kIndent) {
        return lookahead->IntData();
    }
    return lookahead->location.column;
}

unique_ptr<ast::Statement> Parser::Statement() {
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead)
        return nullptr;
    switch (lookahead->type) {
    case Token::Type::kExport:
    case Token::Type::kDefine: return Definition();
    case Token::Type::kReturn: return Return();
    case Token::Type::kIf: return If();
    case Token::Type::kFor: return For();
    case Token::Type::kSwitch: return Switch();
    case Token::Type::kBreak: return Break();
    case Token::Type::kContinue: return Continue();
    default: return ExpressionOrAssignment();
    }
    return nullptr;
}

unique_ptr<ast::Statement> Parser::Definition() {
    return nullptr;
}

unique_ptr<ast::ReturnStmt> Parser::Return() {
    return nullptr;
}
unique_ptr<ast::IfStmt> Parser::If() {
    return nullptr;
}

unique_ptr<ast::ForStmt> Parser::For() {
    return nullptr;
}

unique_ptr<ast::SwitchStmt> Parser::Switch() {
    return nullptr;
}

unique_ptr<ast::LoopControlStmt> Parser::Break() {
    return nullptr;
}

unique_ptr<ast::LoopControlStmt> Parser::Continue() {
    return nullptr;
}

unique_ptr<ast::Statement> Parser::ExpressionOrAssignment() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::Expression() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::StartWithIdentifier() {
    return nullptr;
}

unique_ptr<ast::FunctionCall> Parser::FunctionCall() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::UnaryOperation() {
    return nullptr;
}

unique_ptr<ast::Indexing> Parser::Indexing() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::AtomExpr() {
    return nullptr;
}

unique_ptr<ast::Command> Parser::Command() {
    return nullptr;
}

unique_ptr<ast::Literal> Parser::MapArrayLiteral() {
    return nullptr;
}

unique_ptr<ast::TypeExpr> Parser::TypeExpression() {
    return nullptr;
}

unique_ptr<ast::Literal> Parser::Literal() {
    return nullptr;
}

unique_ptr<ast::StringLit> Parser::StringLiteral() {
    return nullptr;
}

unique_ptr<ast::PathLit> Parser::PathLiteral() {
    return nullptr;
}

} // namespace parser
} // namespace sushi