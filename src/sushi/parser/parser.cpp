#include "sushi/parser/parser.h"
#include "boost/optional.hpp"
#include "boost/variant.hpp"
#include "sushi/ast.h"
#include "sushi/parser/detail/lexer-util.h"
#include "sushi/parser/detail/token-util.h"
#include "sushi/util/optional.h"
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
using util::Has;

namespace parser {

using namespace detail;
using ErrorT = Error::Type;

ast::Program Parser::Program(bool emptyable) {
    int indent = NextStatementIndent();
    auto loc = LookaheadAsToken(s_.lexer, false);
    if (indent <= s_.CurrentIndent()) {
        return {};
    }
    auto p = WithBlock(indent, &Parser::Block);
    if (p.statements.empty() and not emptyable)
        s_.RecordError(ErrorT::kEmptyBlock, loc);
    return p;
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

int Parser::NextStatementIndent() {
    Optional(s_.lexer, TokenT::kLineBreak, false);
    auto lookahead = Lookahead(s_.lexer, false);
    if (not lookahead) {
        return -1;
    }
    if (lookahead->type == TokenT::kIndent) {
        return lookahead->IntData();
    }
    return lookahead->location.column - 1;
}

unique_ptr<ast::Statement> Parser::Statement() {
    auto lookahead = SkipSpaceLookahead(s_.lexer);
    if (not lookahead) return nullptr;
    switch (lookahead->type) {
    case TokenT::kExport:
    case TokenT::kDefine: return Definition();
    case TokenT::kReturn: return Return();
    case TokenT::kIf: return If();
    case TokenT::kFor: return For();
    case TokenT::kSwitch: return Switch();
    case TokenT::kBreak:
    case TokenT::kContinue: return LoopControlStmt();
    default: return ExpressionOrAssignment();
    }
    return nullptr;
}

optional<unique_ptr<ast::TypeExpr>> Parser::OptionalTypeInDef() {
    if (not Optional(s_.lexer, TokenT::kColon, true))
        return unique_ptr<ast::TypeExpr>{};
    auto type =
        WithRecovery(&Parser::TypeExpression, {TokenT::kSingleEq}, true, false);
    if (not type) return none;
    return std::move(type);
}

std::unique_ptr<ast::VariableDef>
Parser::VariableDef(bool is_export, optional<std::string> name) {
    auto type = OptionalTypeInDef();
    if (s_.AssertLookahead(TokenT::kSingleEq, true)) {
        auto expr = Expression();
        if (expr) {
            if (AssertStatementEnd() and name and type)
                return make_unique<ast::VariableDef>(
                    is_export, *name, std::move(*type), std::move(expr));
            else
                return nullptr;
        }
    }
    return RecoverFromStatement();
}

std::vector<ast::FunctionDef::Parameter> Parser::ParameterList() {
    if (SkipSpaceNext(s_.lexer)->type == TokenT::kUnit) return {};
    if (Optional(s_.lexer, TokenT::kRParen, true)) return {};
    std::vector<ast::FunctionDef::Parameter> result;
    for (;;) {
        // buggy: can't support empty list
        auto ident = s_.AssertLookahead(TokenT::kIdent, true);
        auto colon = s_.AssertLookahead(TokenT::kColon);
        auto type = WithRecovery(
            &Parser::TypeExpression, {TokenT::kComma, TokenT::kRParen}, true);
        if (ident and colon and type)
            result.push_back({ident->StrData(), std::move(type)});
        if (not Optional(s_.lexer, TokenT::kComma, true)) break;
    }
    Optional(s_.lexer, TokenT::kRParen, true);
    return result;
}

std::unique_ptr<ast::FunctionDef>
Parser::FunctionDef(bool is_export, optional<std::string> name) {
    SUSHI_PARSER_EXIT_LOOP(s_);
    auto params = ParameterList();
    auto ret_type = OptionalTypeInDef();
    auto eq = s_.AssertLookahead(TokenT::kSingleEq, true);
    auto body = Program();
    if (ret_type and eq and name)
        return make_unique<ast::FunctionDef>(
            is_export, *name, std::move(params), std::move(*ret_type),
            std::move(body));
    return nullptr;
}

unique_ptr<ast::Statement> Parser::Definition() {
    using namespace sushi::util::monadic_optional;
    auto export_ = Optional(s_.lexer, TokenT::kExport);
    auto define = s_.AssertLookahead(TokenT::kDefine);
    auto ident = s_.AssertLookahead(TokenT::kIdent);
    if (not define) ident = none;
    auto ident_name = ident > [](const Token &t) { return t.StrData(); };
    auto l = LookaheadAsToken(s_.lexer, true);
    if (l.type == TokenT::kUnit or l.type == TokenT::kLParen)
        return FunctionDef(static_cast<bool>(export_), ident_name);
    return VariableDef(static_cast<bool>(export_), ident_name);
}

unique_ptr<ast::ReturnStmt> Parser::Return() {
    s_.lexer.Next();
    if (OptionalStatementEnd()) return make_unique<ast::ReturnStmt>(nullptr);
    auto expr = Expression();
    if (expr == nullptr) return RecoverFromStatement();
    if (not AssertStatementEnd()) return nullptr;
    return make_unique<ast::ReturnStmt>(std::move(expr));
}

unique_ptr<ast::Expression> Parser::Condition() {
    auto cond = WithRecovery(
        &Parser::Expression, {TokenT::kColon, TokenT::kLineBreak}, false);
    s_.LineBreakOr(TokenT::kColon);
    return cond;
}

ast::Program Parser::Else() {
    if (LookaheadAsToken(s_.lexer, false).type == TokenT::kIf) {
        auto elif = If();
        if (elif == nullptr) return {};
        std::vector<std::unique_ptr<ast::Statement>> stmts;
        stmts.push_back(std::move(elif));
        return ast::Program{std::move(stmts)};
    }
    Optional(s_.lexer, TokenT::kColon, true);
    return Program();
}

ast::Program Parser::OptionalElse() {
    if (not OptionalLookahead(s_.lexer, TokenT::kElse, true)) return {};
    auto indent = NextStatementIndent();
    if (indent < s_.CurrentIndent()) return {};
    auto else_ = SkipSpaceNext(s_.lexer);
    if (indent > s_.CurrentIndent()) {
        return WithWrongIndentBlock(indent, &Parser::Else);
    }
    return Else();
}

unique_ptr<ast::IfStmt> Parser::If() {
    SkipSpaceNext(s_.lexer);
    auto cond = Condition();
    auto true_body = Program();
    ast::Program false_body = OptionalElse();
    if (cond == nullptr or true_body.statements.empty()) return nullptr;
    return make_unique<ast::IfStmt>(
        std::move(cond), std::move(true_body), std::move(false_body));
}

optional<ast::ForStmt::Condition> Parser::LoopCondition() {
    std::string ident;
    if (LookaheadAsToken(s_.lexer, false, 1).type == TokenT::kIdent and
        LookaheadAsToken(s_.lexer, false, 2).type == TokenT::kIn) {
        ident = s_.lexer.Next()->StrData();
        s_.lexer.Next();
    }
    auto cond = Condition();
    if (not cond) return none;
    return ast::ForStmt::Condition{ident, std::move(cond)};
}

unique_ptr<ast::ForStmt> Parser::For() {
    SkipSpaceNext(s_.lexer);
    SUSHI_PARSER_ENTER_LOOP(s_);
    auto cond = LoopCondition();
    auto body = Program();
    if (not cond or body.statements.empty()) return nullptr;
    return make_unique<ast::ForStmt>(std::move(*cond), std::move(body));
}

boost::optional<ast::SwitchStmt::Case> Parser::Case() {
    ast::SwitchStmt::Case case_;
    if (Optional(s_.lexer, TokenT::kDefault, true)) {
        Optional(s_.lexer, TokenT::kColon, true);
        auto p = Program();
        if (p.statements.empty()) return none;
        case_.body = std::move(p);
    } else if (Optional(s_.lexer, TokenT::kCase, true)) {
        auto cond = Condition();
        if (cond == nullptr) return none;
        case_.condition = std::move(cond);
        case_.body = Program(true);
    }
    return std::move(case_);
}

std::vector<ast::SwitchStmt::Case> Parser::Cases() {
    std::vector<ast::SwitchStmt::Case> cases;
    for (optional<const Token &> l;
         (l = SkipSpaceLookahead(s_.lexer)) and
         (l->type == TokenT::kCase or l->type == TokenT::kDefault);) {
        auto indent = NextStatementIndent();
        if (indent < s_.CurrentIndent()) break;
        if (indent > s_.CurrentIndent())
            WithWrongIndentBlock(indent, &Parser::Case);
        else if (auto case_ = Case())
            cases.push_back(std::move(*case_));
    }
    return cases;
}

unique_ptr<ast::SwitchStmt> Parser::Switch() {
    auto switch_ = SkipSpaceNext(s_.lexer);
    auto switched = WithRecovery(
        &Parser::Expression, {TokenT::kCase, TokenT::kDefault}, true);
    auto l = SkipSpaceLookahead(s_.lexer);
    if (not l or (l->type != TokenT::kCase and l->type != TokenT::kDefault))
        return nullptr;

    auto indent = NextStatementIndent();
    if (indent < s_.CurrentIndent())
        return s_.RecordError(
            ErrorT::kExpectToken,
            {TokenT::kCase, s_.lexer.Lookahead()->location, 0});

    auto cases = WithBlock(indent, &Parser::Cases);
    if (switched == nullptr or cases.empty()) return nullptr;
    return make_unique<ast::SwitchStmt>(std::move(switched), std::move(cases));
}

unique_ptr<ast::LoopControlStmt> Parser::LoopControlStmt() {
    auto ctrl = *SkipSpaceNext(s_.lexer);
    auto v = LoopControlTokToAst(ctrl.type);
    if (OptionalStatementEnd()) return make_unique<ast::LoopControlStmt>(v, 1);

    auto level = s_.AssertLookahead(TokenT::kIntLit);
    if (not level) return RecoverFromStatement();
    if (not AssertStatementEnd()) return nullptr;
    auto lv = level->IntData();
    if (lv < s_.LoopLevel())
        return s_.RecordError(ErrorT::kInvalidLoopLevel, *level);
    return make_unique<ast::LoopControlStmt>(v, lv);
}

unique_ptr<ast::Statement> Parser::ExpressionOrAssignment() {
    unique_ptr<ast::Statement> ret;
    auto expr = Expression();
    if (Optional(s_.lexer, TokenT::kSingleEq, false)) {
        auto value = Expression();
        if (expr != nullptr and value != nullptr)
            ret =
                make_unique<ast::Assignment>(std::move(expr), std::move(value));
    } else {
        ret = std::move(expr);
    }
    if (not AssertStatementEnd()) return nullptr;
    return ret;
}

unique_ptr<ast::Expression> Parser::PrimaryExpr() {
    auto l = *SkipSpaceLookahead(s_.lexer);
    if (l.type == TokenT::kIdent) return StartWithIdentifier();
    if (l.type == TokenT::kLBrace or l.type == TokenT::kLParen or
        IsLiteral(l.type))
        return AtomExpr();
    if (l.type == TokenT::kExclamation) return CommandLike();
    if (IsLiteral(l.type)) return AtomExpr();
    if (IsUnaryOp(l.type)) return UnaryOperation();
    return s_.RecordErrorOnLookahead(ErrorT::kExpectExpression, false);
}

unique_ptr<ast::Expression> Parser::Expression() {
    auto lhs = PrimaryExpr();
    if (lhs == nullptr) return nullptr;
    return PrecedenceClimb(std::move(lhs), -1);
}

unique_ptr<ast::Expression>
Parser::PrecedenceClimb(unique_ptr<ast::Expression> lhs, int min_prec) {
    for (auto l = SkipSpaceLookahead(s_.lexer);
         l and IsBinaryOp(l->type) and BinaryOpPrec(l->type) >= min_prec;) {
        auto op = *SkipSpaceNext(s_.lexer);
        auto rhs = PrimaryExpr();
        for (; (l = SkipSpaceLookahead(s_.lexer)) and IsBinaryOp(l->type) and
               BinaryOpPrec(l->type) > BinaryOpPrec(op.type);) {
            rhs = PrecedenceClimb(std::move(rhs), BinaryOpPrec(l->type));
        }
        if (rhs == nullptr)
            lhs = nullptr;
        else if (lhs != nullptr)
            lhs = make_unique<ast::BinaryExpr>(
                std::move(lhs), BinOpTokenToOperator(op.type), std::move(rhs));
    }
    return lhs;
}

unique_ptr<ast::Expression> Parser::StartWithIdentifier() {
    auto l2 = SkipSpaceLookahead(s_.lexer, 2);
    if (not l2) return Variable();
    if (l2->type == TokenT::kLBracket) return AtomExpr();
    if (IsAtomExprLookahead(l2->type)) return CommandLike();
    return Variable();
}

namespace {

unique_ptr<ast::CommandLike>
FromPipeline(std::vector<unique_ptr<ast::CommandLike>> pipeline) {
    unique_ptr<ast::CommandLike> result = nullptr;
    for (; not pipeline.empty();) {
        pipeline.back()->pipe_next = std::move(result);
        result = std::move(pipeline.back());
        pipeline.pop_back();
    }
    return result;
}

} // namespace

optional<ast::Redirection> Parser::RedirectTo() {
    std::unique_ptr<ast::Expression> ext;
    bool fail = false;
    if (not Optional(s_.lexer, TokenT::kHere, true)) {
        ext = AtomExpr();
        fail = ext == nullptr;
    }
    bool append = static_cast<bool>(Optional(s_.lexer, TokenT::kAppend));
    if (fail) return none;
    return ast::Redirection(
        ast::FdLit::Value::kStdout, ast::Redirection::Direction::kOut,
        std::move(ext), append);
}

optional<ast::Redirection> Parser::RedirectFrom() {
    auto expr = AtomExpr();
    if (not expr) return none;
    return ast::Redirection(
        ast::FdLit::Value::kStderr, ast::Redirection::Direction::kIn,
        std::move(expr), false);
}

optional<ast::Redirection> Parser::RedirectItem() {
    using namespace util::monadic_optional;
    auto me_tok = Optional(s_.lexer, IsFdLiteral, true);
    auto me = me_tok > [](const Token &t) { return FdLiteralToFd(t.type); };
    if (Optional(s_.lexer, TokenT::kTo, true))
        return RedirectTo() > [&](ast::Redirection redir) {
            redir.me = me.value_or(ast::FdLit::Value::kStdout);
            return redir;
        };
    if (Optional(s_.lexer, TokenT::kFrom, true))
        return RedirectFrom() > [&](ast::Redirection redir) {
            redir.me = me.value_or(ast::FdLit::Value::kStdin);
            return redir;
        };
    s_.ExpectToken(TokenT::kTo);
    return none;
}

optional<std::vector<ast::Redirection>> Parser::Redirections() {
    auto redir = Optional(s_.lexer, TokenT::kRedirect, true);
    if (not redir) return std::vector<ast::Redirection>{};
    std::vector<ast::Redirection> redirs;
    bool fail = false;
    for (;;) {
        auto item = RedirectItem();
        if (not item)
            fail = true;
        else
            redirs.push_back(std::move(*item));
        if (not Optional(s_.lexer, TokenT::kComma)) break;
    }
    if (redirs.empty()) {
        s_.RecordErrorOnLookahead(ErrorT::kExpectRedirItem);
        fail = true;
    }
    if (fail) return none;
    return redirs;
}

std::unique_ptr<ast::CommandLike> Parser::AssertCommandLike() {
    auto l = *SkipSpaceLookahead(s_.lexer);
    if (l.type == TokenT::kExclamation) return Command();
    if (l.type == TokenT::kIdent) return FunctionCall();
    Recover({TokenT::kSemicolon, TokenT::kRedirect, TokenT::kLineBreak,
             TokenT::kPipe});
    Optional(s_.lexer, TokenT::kSemicolon);
    return s_.RecordError(ErrorT::kExpectCommand, l);
}

unique_ptr<ast::CommandLike> Parser::SingleCommandLike() {
    unique_ptr<ast::CommandLike> cmd = AssertCommandLike();

    optional<std::vector<ast::Redirection>> redir = Redirections();
    if (cmd == nullptr or not redir) return nullptr;

    cmd->redirs = std::move(*redir);
    return cmd;
}

unique_ptr<ast::CommandLike> Parser::CommandLike() {
    std::vector<unique_ptr<ast::CommandLike>> pipeline;
    bool fail = false;
    for (optional<const Token &> l; (l = SkipSpaceLookahead(s_.lexer));) {
        auto cmd = SingleCommandLike();
        fail = fail or cmd == nullptr;
        pipeline.push_back(std::move(cmd));
        auto pipe = Optional(s_.lexer, TokenT::kPipe, true);
        if (not pipe) break;
        if (not s_.lexer.Lookahead())
            return s_.RecordError(ErrorT::kExpectCommand, *pipe);
    }
    if (fail or pipeline.empty()) return nullptr;
    return FromPipeline(std::move(pipeline));
}

unique_ptr<ast::FunctionCall> Parser::FunctionCall() {
    auto func_name = *SkipSpaceNext(s_.lexer);
    bool fail = false;
    std::vector<std::unique_ptr<ast::Expression>> params;
    for (optional<const Token &> l;
         (l = s_.lexer.Lookahead()) and IsAtomExprLookahead(l->type);) {
        auto param = AtomExpr();
        fail = fail or param == nullptr;
        params.push_back(std::move(param));
    }
    if (fail) return nullptr;
    return make_unique<ast::FunctionCall>(
        ast::Identifier{func_name.StrData()}, std::move(params),
        vector<ast::Redirection>{}, nullptr);
}

boost::optional<ast::InterpolatedString> Parser::CommandArg() {
    auto l = *s_.lexer.Lookahead();
    if (l.type == TokenT::kStringLit) {
        auto str = StringLiteral();
        if (str == nullptr) return none;
        return std::move(str->value);
    }
    if (l.type == TokenT::kRawString) {
        s_.lexer.Next();
        return Interpolatable(false);
    }
    if (IsError(l.type)) s_.RecordError(ErrorT::kLexicalError, std::move(l));
    return none;
}

namespace {

unique_ptr<ast::Command> FromCommandArgs(vector<ast::InterpolatedString> args) {
    auto cmd = std::move(args.front());
    args.erase(begin(args));
    return make_unique<ast::Command>(
        std::move(cmd), std::move(args), std::vector<ast::Redirection>{},
        nullptr);
}

} // namespace

unique_ptr<ast::Command> Parser::Command() {
    auto exclamation = SkipSpaceNext(s_.lexer);
    bool fail = false;
    std::vector<ast::InterpolatedString> args;
    for (optional<const Token &> l;
         (l = s_.lexer.Lookahead()) and not IsRawExit(l->type);) {
        auto a = CommandArg();
        fail = fail or not a;
        if (a) args.push_back(std::move(*a));
    }
    SkipStatementEnd();
    if (fail) return nullptr;
    if (args.empty())
        return s_.RecordError(ErrorT::kExpectCommand, std::move(*exclamation));
    return FromCommandArgs(std::move(args));
}

std::unique_ptr<ast::Variable> Parser::Variable() {
    auto ident = *SkipSpaceNext(s_.lexer);
    return make_unique<ast::Variable>(ast::Identifier{ident.StrData()});
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
    return expr;
}

unique_ptr<ast::Expression> Parser::Index() {
    s_.lexer.Next();
    auto expr = Expression();
    if (expr == nullptr) return RecoverFromExpression({TokenT::kRBracket});
    if (not s_.AssertLookahead(TokenT::kRBracket)) return nullptr;
    return expr;
}

optional<std::vector<unique_ptr<ast::Expression>>> Parser::Indices() {
    std::vector<unique_ptr<ast::Expression>> indices;
    bool fail = false;
    for (; LookaheadAsToken(s_.lexer, false).type == TokenT::kLBracket;) {
        auto index = Index();
        fail = fail or index == nullptr;
        indices.push_back(std::move(index));
    }
    if (fail) return none;
    return std::move(indices);
}

unique_ptr<ast::Expression> Parser::AtomExpr() {
    auto l = SkipSpaceLookahead(s_.lexer);
    unique_ptr<ast::Expression> expr;
    if (l->type == TokenT::kLParen)
        expr = ParenExpr();
    else if (l->type == TokenT::kIdent)
        expr = Variable();
    else if (IsLiteral(l->type))
        expr = Literal();
    else if (l->type == TokenT::kLBrace)
        expr = MapArrayLiteral();

    auto indices = Indices();
    if (not indices) return nullptr;
    for (auto &index : *indices)
        expr = make_unique<ast::Indexing>(std::move(expr), std::move(index));

    return expr;
}

optional<std::vector<std::unique_ptr<ast::Expression>>>
Parser::ArrayItems(std::vector<std::unique_ptr<ast::Expression>> init) {
    bool fail = false;
    for (optional<const Token &> l; (l = SkipSpaceLookahead(s_.lexer));) {
        auto elem = WithRecovery(
            &Parser::Expression, {TokenT::kComma, TokenT::kRBrace});
        if (elem == nullptr) fail = true;
        init.push_back(std::move(elem));
        if (not Optional(s_.lexer, TokenT::kComma, true)) break;
    }
    if (fail) return none;
    return std::move(init);
}

optional<std::vector<ast::MapLit::MapItem>>
Parser::MapItems(std::vector<ast::MapLit::MapItem> init) {
    bool fail = false;
    for (optional<const Token &> l; (l = SkipSpaceLookahead(s_.lexer));) {
        auto t = *l;
        auto key = WithRecovery(
            &Parser::Expression,
            {TokenT::kColon, TokenT::kComma, TokenT::kRBrace});
        if (Optional(s_.lexer, TokenT::kColon, true)) {
            auto value = WithRecovery(
                &Parser::Expression, {TokenT::kComma, TokenT::kRBrace});
            if (key == nullptr or value == nullptr) fail = true;
            init.push_back({std::move(key), std::move(value)});
        } else {
            s_.RecordError(ErrorT::kExpectMappedValue, t);
        }
        if (not Optional(s_.lexer, TokenT::kComma, true)) break;
    }
    if (fail) return none;
    return std::move(init);
}

std::unique_ptr<ast::MapLit>
Parser::ConfirmedMapLiteral(std::unique_ptr<ast::Expression> expr) {
    auto v = WithRecovery(
        &Parser::Expression, {TokenT::kRBrace, TokenT::kComma}, true);
    std::vector<ast::MapLit::MapItem> kvs;
    if (v != nullptr and expr != nullptr)
        kvs.push_back({std::move(expr), std::move(v)});
    if (Optional(s_.lexer, TokenT::kRBrace, true)) {
        if (kvs.empty()) return nullptr;
        return make_unique<ast::MapLit>(std::move(kvs));
    } else if (Optional(s_.lexer, TokenT::kComma, true)) {
        auto items = MapItems(std::move(kvs));
        if (not s_.AssertLookahead(TokenT::kRBrace))
            return RecoverFromExpression({TokenT::kRBrace});
        else if (items)
            return make_unique<ast::MapLit>(std::move(*items));
    }
    return nullptr;
}

std::unique_ptr<ast::ArrayLit>
Parser::ConfirmedArrayLiteral(std::unique_ptr<ast::Expression> expr) {
    auto items = ArrayItems();
    if (not s_.AssertLookahead(TokenT::kRBrace))
        return RecoverFromExpression({TokenT::kRBrace});
    if (not items or expr == nullptr) return nullptr;
    items->insert(begin(*items), std::move(expr));
    return make_unique<ast::ArrayLit>(std::move(*items));
}

std::unique_ptr<ast::Literal> Parser::NonEmptyMapArray() {
    std::vector<TokenT> nexts{TokenT::kRBrace, TokenT::kColon, TokenT::kComma};
    auto expr = WithRecovery(&Parser::Expression, nexts, true);
    if (Optional(s_.lexer, TokenT::kComma, true))
        return ConfirmedArrayLiteral(std::move(expr));
    else if (Optional(s_.lexer, TokenT::kColon, true))
        return ConfirmedMapLiteral(std::move(expr));
    else if (Optional(s_.lexer, TokenT::kRBrace, true)) {
        std::vector<unique_ptr<ast::Expression>> singleton;
        singleton.push_back(std::move(expr));
        return make_unique<ast::ArrayLit>(std::move(singleton));
    }
    return nullptr;
}

unique_ptr<ast::Literal> Parser::MapArrayLiteral() {
    SkipSpaceNext(s_.lexer);
    if (Optional(s_.lexer, TokenT::kRBrace, true))
        return make_unique<ast::ArrayLit>(
            std::vector<std::unique_ptr<ast::Expression>>{});
    return NonEmptyMapArray();
}

std::unique_ptr<ast::TypeExpr> Parser::TypeInParen(const Token &lparen) {
    auto t = TypeExpression();
    if (not t or not s_.AssertLookahead(TokenT::kRParen, true))
        return RecoverFromExpression({TokenT::kLParen});
    return t;
}

namespace {

unique_ptr<ast::FunctionType>
FromFunctionTypeParams(std::vector<unique_ptr<ast::TypeExpr>> types) {
    auto ret = std::move(types.front());
    types.erase(begin(types));
    return make_unique<ast::FunctionType>(std::move(types), std::move(ret));
}

} // namespace

unique_ptr<ast::FunctionType> Parser::FunctionType(const Token &func) {
    std::vector<unique_ptr<ast::TypeExpr>> types;
    bool fail = false;
    for (optional<const Token &> l;
         (l = s_.lexer.Lookahead()) and
         (l->type == TokenT::kLParen or IsSimpleType(l->type));) {
        auto t = TypeExpression();
        fail = fail or t == nullptr;
        types.push_back(std::move(t));
    }
    if (fail) return nullptr;
    if (types.empty()) return s_.RecordError(ErrorT::kWrongTypeKind, func);
    return FromFunctionTypeParams(std::move(types));
}

optional<type::BuiltInAtom::Type> Parser::AssertSimpleType() {
    auto t = s_.AssertLookahead(IsSimpleType, ErrorT::kExpectSimpleType, false);
    if (not t) return none;
    return TypeTokenToType(t->type);
}

std::unique_ptr<ast::ArrayType> Parser::ArrayType(const Token &arr) {
    auto e = AssertSimpleType();
    if (not e) return nullptr;
    return make_unique<ast::ArrayType>(*e);
}

std::unique_ptr<ast::MapType> Parser::MapType(const Token &map) {
    auto k = AssertSimpleType(), v = AssertSimpleType();
    if (not k or not v) return nullptr;
    return make_unique<ast::MapType>(*k, *v);
}

unique_ptr<ast::TypeExpr> Parser::TypeExpression() {
    auto t = s_.AssertLookahead(IsTypeLookahead, ErrorT::kExpectType, true);
    if (not t) return nullptr;
    auto l = *t;
    if (IsSimpleType(l.type)) {
        return make_unique<ast::TypeLit>(TypeTokenToType(l.type));
    }
    if (l.type == TokenT::kLParen) return TypeInParen(l);
    if (l.type == TokenT::kArray) return ArrayType(l);
    if (l.type == TokenT::kMap) return MapType(l);
    if (l.type == TokenT::kFunction) return FunctionType(l);
    return nullptr;
}

unique_ptr<ast::Literal> Parser::Literal() {
    auto t = *SkipSpaceNext(s_.lexer);
    if (t.type == TokenT::kIntLit) return make_unique<ast::IntLit>(t.IntData());
    if (t.type == TokenT::kUnit) return make_unique<ast::UnitLit>();
    if (t.type == TokenT::kCharLit)
        return make_unique<ast::CharLit>(char(t.IntData()));
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
        return util::Has(stops, t);
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
    SkipStatementEnd();
    return true;
}

void Parser::SkipStatementEnd() {
    Optional(s_.lexer, TokenT::kSemicolon, false);
    Optional(s_.lexer, TokenT::kLineBreak, false);
}

nullptr_t Parser::RecoverFromStatement() {
    Recover([](TokenT t) {
        return t == TokenT::kLineBreak or t == TokenT::kIndent;
    });
    SkipStatementEnd();
    return nullptr;
}

nullptr_t Parser::RecoverFromExpression(std::vector<lexer::Token::Type> extra) {
    Recover([&extra](TokenT t) {
        return t == TokenT::kLineBreak or
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