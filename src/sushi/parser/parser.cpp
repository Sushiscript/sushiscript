#include "sushi/parser/parser.h"
#include "boost/optional.hpp"
#include "boost/variant.hpp"
#include "sushi/ast.h"
#include "sushi/parser/detail/lexer-util.h"
#include "sushi/parser/detail/token-util.h"
#include <algorithm>

using boost::get;
using boost::none;
using boost::optional;
using std::unique_ptr;
using std::vector;

namespace sushi {

using lexer::Token;
using TokenT = Token::Type;

namespace parser {

using namespace detail;
using ErrorT = Error::Type;

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
    if (lookahead->type != TokenT::kIndent)
        return Statement();

    // type != Type::kIndent
    int indent = lookahead->IntData();
    if (indent < s_.CurrentIndent())
        return boost::none;
    if (indent > s_.CurrentIndent()) {
        s_.RecordError(ErrorT::kUnexpectIndent, *Next(s_.lexer, false));
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
    if (lookahead->type == TokenT::kIndent) {
        return lookahead->IntData();
    }
    return lookahead->location.column;
}

unique_ptr<ast::Statement> Parser::Statement() {
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead)
        return nullptr;
    switch (lookahead->type) {
    case TokenT::kExport:
    case TokenT::kDefine: return Definition();
    case TokenT::kReturn: return Return();
    case TokenT::kIf: return If();
    case TokenT::kFor: return For();
    case TokenT::kSwitch: return Switch();
    case TokenT::kBreak: return Break();
    case TokenT::kContinue: return Continue();
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

unique_ptr<ast::Expression>
PrecedenceClimb(unique_ptr<ast::Expression> lhs, int min_prec) {
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

unique_ptr<ast::Indexing> Parser::Index() {
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

std::unique_ptr<ast::Expression> Parser::InterExpr() {
    auto expr = Expression();
    if (not expr or not s_.AssertLookahead(TokenT::kRBrace, false))
        return Recover({TokenT::kRBrace, TokenT::kLineBreak});
    return expr;
}

bool Parser::InterpolateAction(
    Token t, bool exit_on_err, ast::InterpolatedString &inter_str, bool &err) {
    if (IsError(t.type)) {
        s_.RecordError(ErrorT::kLexicalError, std::move(t));
        err = true;
        return exit_on_err;
    }
    if (t.type == TokenT::kInterDone) {
        return true;
    }
    if (t.type == TokenT::kInterStart) {
        auto expr = InterExpr();
        err = err or expr == nullptr;
        inter_str.Append(std::move(expr));
    } else if (t.type == TokenT::kSegment) {
        inter_str.Append(t.StrData());
    }
    return false;
}

optional<ast::InterpolatedString> Parser::Interpolatable(bool exit_on_err) {
    ast::InterpolatedString inter_str;
    optional<lexer::Token> t;
    bool err = false;
    for (; (t = s_.lexer.Next());) {
        auto finish = InterpolateAction(*t, exit_on_err, inter_str, err);
        if (finish)
            break;
    }
    if (err)
        return none;
    return std::move(inter_str);
}

unique_ptr<ast::StringLit> Parser::StringLiteral() {
    SkipSpaceNext(s_.lexer);
    auto content = Interpolatable(true);
    if (not content)
        return nullptr;
    return std::make_unique<ast::StringLit>(std::move(*content));
}

namespace {

bool IsRelativePath(ast::InterpolatedString &s) {
    std::string start;
    s.Traverse(
        [&start](const std::string &s) {
            if (start.empty())
                start = s;
        },
        [](auto &) {});
    return start.front() == '.';
}

} // namespace

unique_ptr<ast::Literal> Parser::PathLiteral() {
    SkipSpaceNext(s_.lexer);
    auto content = Interpolatable(false);
    if (not content) {
        return nullptr;
    }
    if (IsRelativePath(*content)) {
        return std::make_unique<ast::RelPathLit>(std::move(*content));
    }
    return std::make_unique<ast::PathLit>(std::move(*content));
}

nullptr_t Parser::Recover(std::vector<TokenT> stops) {
    for (optional<TokenT> skipped; (skipped = SkipToken());) {
        if (std::find(begin(stops), end(stops), *skipped) == end(stops))
            break;
    }
    return nullptr;
}

optional<TokenT> Parser::SkipToken() {
    return none;
}

} // namespace parser
} // namespace sushi