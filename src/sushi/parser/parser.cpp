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
using std::make_unique;
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
        if (not ostmt) break;
        if (*ostmt != nullptr) p.statements.push_back(std::move(*ostmt));
    }
    return p;
}
optional<unique_ptr<ast::Statement>> Parser::CurrentBlockStatement() {
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead) return boost::none;
    if (lookahead->type != TokenT::kIndent) return Statement();

    // type != Type::kIndent
    int indent = lookahead->IntData();
    if (indent < s_.CurrentIndent()) return boost::none;
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
    if (not lookahead) return nullptr;
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
    s_.lexer.Next();
    if (OptionalStatementEnd()) return make_unique<ast::ReturnStmt>(nullptr);
    auto expr = Expression();
    if (expr == nullptr) return RecoverFromStatement();
    if (not AssertStatementEnd()) return nullptr;
    return make_unique<ast::ReturnStmt>(std::move(expr));
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
    using V = ast::LoopControlStmt::Value;
    Next(s_.lexer, false);
    if (OptionalStatementEnd())
        return make_unique<ast::LoopControlStmt>(V::kBreak, 1);

    auto level = s_.AssertLookahead(TokenT::kIntLit);
    if (not level) return RecoverFromStatement();
    if (not AssertStatementEnd()) return nullptr;
    return make_unique<ast::LoopControlStmt>(V::kBreak, level->IntData());
}

unique_ptr<ast::LoopControlStmt> Parser::Continue() {
    using V = ast::LoopControlStmt::Value;
    Next(s_.lexer, false);
    if (OptionalStatementEnd())
        return make_unique<ast::LoopControlStmt>(V::kContinue, 1);

    auto level = s_.AssertLookahead(TokenT::kIntLit);
    if (not level) return RecoverFromStatement();
    if (not AssertStatementEnd()) return nullptr;
    return make_unique<ast::LoopControlStmt>(V::kContinue, level->IntData());
}

unique_ptr<ast::Statement> Parser::ExpressionOrAssignment() {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::PrimaryExpr() {
    auto l = *Lookahead(s_.lexer, true);
    if (l.type == TokenT::kIdent) return StartWithIdentifier();
    if (l.type == TokenT::kLBrace) return MapArrayLiteral();
    if (IsUnaryOp(l.type)) return UnaryOperation();
    if (IsLiteral(l.type)) return Literal();
    s_.RecordError(ErrorT::kUnexpectToken, l);
    return nullptr;
}

unique_ptr<ast::Expression> Parser::Expression() {
    auto lhs = PrimaryExpr();
    if (lhs == nullptr) return nullptr;
    return PrecedenceClimb(std::move(lhs), -1);
}

unique_ptr<ast::Expression>
PrecedenceClimb(unique_ptr<ast::Expression> lhs, int min_prec) {
    return nullptr;
}

unique_ptr<ast::Expression> Parser::StartWithIdentifier() {
    return nullptr;
}

std::vector<ast::Redirection> Redirections() {
    return {};
}

unique_ptr<ast::CommandLike> CommandLike() {
    return nullptr;
}

unique_ptr<ast::FunctionCall> Parser::FunctionCall() {
    return nullptr;
}

unique_ptr<ast::Command> Parser::Command() {
    s_.lexer.Next();
    s_.lexer.NewContext() return nullptr;
}

unique_ptr<ast::Expression> Parser::UnaryOperation() {
    auto op = Optional(s_.lexer, IsUnaryOp, true);
    if (not op) return AtomExpr();
    auto expr = UnaryOperation();
    if (not expr) return nullptr;
    return make_unique<ast::UnaryExpr>(
        std::move(expr), UnaryOpTokenToOperator(op->type));
}

std::unique_ptr<ast::Expression> Parser::ParenExpr() {
    SkipSpaceNext(s_.lexer);
    auto expr = Expression();
    if (expr == nullptr or not s_.AssertLookahead(TokenT::kRParen))
        return RecoverFromExpression({TokenT::kRParen});

    if (not s_.AssertLookahead(TokenT::kRParen)) return nullptr;
    return expr;
}

unique_ptr<ast::Expression> Parser::Index() {
    s_.lexer.Next();
    auto expr = Expression();
    if (expr == nullptr) return RecoverFromExpression({TokenT::kRBracket});
    if (not s_.AssertLookahead(TokenT::kRBracket)) return nullptr;
    return expr;
}

unique_ptr<ast::Expression> Parser::AtomExpr() {
    auto l = s_.lexer.Lookahead();
    unique_ptr<ast::Expression> expr;
    if (l->type == TokenT::kLParen) expr = ParenExpr();
    if (l->type == TokenT::kIdent)
        expr = make_unique<ast::Variable>(ast::Identifier{l->StrData()});

    for (; (l = s_.lexer.Lookahead()) and l->type == TokenT::kLBracket;) {
        auto index = Index();
        if (expr != nullptr and index != nullptr)
            expr =
                make_unique<ast::Indexing>(std::move(expr), std::move(index));
        else
            expr = nullptr;
    }
    return expr;
}

unique_ptr<ast::Literal> Parser::MapArrayLiteral() {
    return nullptr;
}

unique_ptr<ast::TypeExpr> Parser::TypeExpression() {
    return nullptr;
}

unique_ptr<ast::Literal> Parser::Literal() {
    auto t = *SkipSpaceNext(s_.lexer);
    if (t.type == TokenT::kIntLit) return make_unique<ast::IntLit>(t.IntData());
    if (t.type == TokenT::kUnitLit) return make_unique<ast::UnitLit>();
    if (IsBoolLiteral(t.type))
        return make_unique<ast::BoolLit>(BoolLitToBool(t.type));
    if (IsFdLiteral(t.type))
        return make_unique<ast::FdLit>(FdLiteralToFd(t.type));
    if (t.type == TokenT::kStringLit) return StringLiteral();
    if (t.type == TokenT::kPathLit) return PathLiteral();
    return nullptr;
}

std::unique_ptr<ast::Expression> Parser::InterExpr() {
    auto expr = Expression();
    if (not expr) return RecoverFromExpression({TokenT::kRBrace});
    if (not s_.AssertLookahead(TokenT::kRBrace, false)) return nullptr;
    return expr;
}

bool Parser::InterpolateAction(
    Token t, bool exit_on_err, ast::InterpolatedString &inter_str, bool &err) {
    if (IsError(t.type)) {
        s_.RecordError(ErrorT::kLexicalError, std::move(t));
        err = true;
        return exit_on_err;
    }
    if (t.type == TokenT::kInterDone) return true;

    if (t.type == TokenT::kInterStart) {
        auto expr = InterExpr();
        err = err or expr == nullptr;
        inter_str.Append(std::move(expr));
        s_.lexer.DestoryContext();
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
        if (finish) break;
    }
    if (err) return none;
    return std::move(inter_str);
}

unique_ptr<ast::StringLit> Parser::StringLiteral() {
    SkipSpaceNext(s_.lexer);
    auto content = Interpolatable(true);
    if (not content) return nullptr;
    return std::make_unique<ast::StringLit>(std::move(*content));
}

namespace {

bool IsRelativePath(ast::InterpolatedString &s) {
    std::string start;
    s.Traverse(
        [&start](const std::string &s) {
            if (start.empty()) start = s;
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
    return Recover([stops = std::move(stops)](lexer::Token::Type t) {
        return std::find(begin(stops), end(stops), t) != end(stops);
    });
}

nullptr_t Parser::Recover(std::function<bool(lexer::Token::Type)> p) {
    for (optional<Token> l; (l = s_.lexer.Lookahead());) {
        if (p(l->type)) break;
        SkipToken();
    }
    return nullptr;
}
bool Parser::OptionalStatementEnd() {
    auto l = s_.lexer.Lookahead();
    if (not l) return true;
    if (not IsStatementEnd(l->type)) return false;
    SkipStatementEnd();
    return true;
}

bool Parser::AssertStatementEnd() {
    auto l = s_.lexer.Lookahead();
    if (not l) return true;
    if (not IsStatementEnd(l->type)) {
        s_.RecordError(
            Error::Type::kExpectToken, {TokenT::kSemicolon, l->location, 0});
        RecoverFromStatement();
        return false;
    }
    return true;
}

void Parser::SkipStatementEnd() {
    Optional(s_.lexer, TokenT::kSemicolon, false);
    Optional(s_.lexer, TokenT::kLineBreak, false);
}

nullptr_t Parser::RecoverFromStatement() {
    Recover(IsStatementEnd);
    SkipStatementEnd();
    return nullptr;
}

nullptr_t Parser::RecoverFromExpression(std::vector<lexer::Token::Type> extra) {
    Recover([&extra](TokenT t) {
        return IsStatementEnd(t) or
               std::find(begin(extra), end(extra), t) != end(extra);
    });
    for (auto t : extra) Optional(s_.lexer, t, false);
    return nullptr;
}

nullptr_t RecoverFromExpression(std::vector<lexer::Token::Type>);
optional<TokenT> Parser::SkipToken() {
    auto t = s_.lexer.Next();
    if (not t) return none;
    if (IsInterpolatable(t->type))
        Interpolatable(t->type == TokenT::kStringLit);
    return t->type;
}

} // namespace parser
} // namespace sushi