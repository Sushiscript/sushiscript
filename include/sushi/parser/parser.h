#ifndef SUSHI_PARSER_PARSER_H_
#define SUSHI_PARSER_PARSER_H_

#include "sushi/ast.h"
#include "sushi/lexer.h"
#include "sushi/parser/detail/parser-state.h"
#include "sushi/parser/error.h"
#include "sushi/util/container.h"

#include <functional>
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
    Parser(lexer::Lexer lexer) : s_(std::move(lexer)) {}

    ParsingResult Parse() {
        SUSHI_PARSER_NEW_BLOCK(s_, -1);
        auto p = Program();
        return {std::move(p), std::move(s_.errors)};
    }

  private:
    template <typename T>
    T WithBlock(int indent, T (Parser::*parse)()) {
        SUSHI_PARSER_NEW_BLOCK(s_, indent);
        auto ret = (this->*parse)();
        return ret;
    }

    template <typename T>
    T WithWrongIndentBlock(int indent, T (Parser::*parse)()) {
        s_.RecordError(
            Error::Type::kUnexpectIndent,
            detail::LookaheadAsToken(s_.lexer, false));
        WithBlock(indent, parse);
        return T{};
    }

    ast::Program Program(bool emptyable = false);

    ast::Program Block();

    int NextStatementIndent();

    boost::optional<std::unique_ptr<ast::Statement>> CurrentBlockStatement();

    std::unique_ptr<ast::Statement> Statement();

    std::unique_ptr<ast::VariableDef>
    VariableDef(bool is_export, boost::optional<std::string> name);

    std::vector<ast::FunctionDef::Parameter> ParameterList();

    std::unique_ptr<ast::FunctionDef>
    FunctionDef(bool is_export, boost::optional<std::string> name);

    boost::optional<std::unique_ptr<ast::TypeExpr>> OptionalTypeInDef();

    std::unique_ptr<ast::Statement> Definition();

    std::unique_ptr<ast::ReturnStmt> Return();

    std::unique_ptr<ast::Expression> Condition();

    ast::Program Else();

    ast::Program OptionalElse();

    std::unique_ptr<ast::IfStmt> If();

    boost::optional<ast::ForStmt::Condition> LoopCondition();

    std::unique_ptr<ast::ForStmt> For();

    std::vector<ast::SwitchStmt::Case> Cases();

    boost::optional<ast::SwitchStmt::Case> Case();

    std::unique_ptr<ast::SwitchStmt> Switch();

    std::unique_ptr<ast::LoopControlStmt> Break();

    std::unique_ptr<ast::LoopControlStmt> Continue();

    std::unique_ptr<ast::Statement> ExpressionOrAssignment();

    std::unique_ptr<ast::Expression> PrimaryExpr();

    std::unique_ptr<ast::Expression> Expression();

    std::unique_ptr<ast::Expression>
    PrecedenceClimb(std::unique_ptr<ast::Expression> lhs, int min_prec);

    std::unique_ptr<ast::Expression> StartWithIdentifier();

    boost::optional<ast::Redirection> RedirectTo();

    boost::optional<ast::Redirection> RedirectFrom();

    boost::optional<ast::Redirection> RedirectItem();

    boost::optional<std::vector<ast::Redirection>> Redirections();

    std::unique_ptr<ast::CommandLike> AssertCommandLike();

    std::unique_ptr<ast::CommandLike> SingleCommandLike();

    std::unique_ptr<ast::CommandLike> CommandLike();

    std::unique_ptr<ast::FunctionCall> FunctionCall();

    boost::optional<ast::InterpolatedString> CommandArg();

    std::unique_ptr<ast::Command> Command();

    std::unique_ptr<ast::Variable> Variable();

    std::unique_ptr<ast::Expression> UnaryOperation();

    std::unique_ptr<ast::Expression> ParenExpr();

    std::unique_ptr<ast::Expression> Index();

    boost::optional<std::vector<std::unique_ptr<ast::Expression>>> Indices();

    std::unique_ptr<ast::Expression> AtomExpr();

    boost::optional<std::vector<std::unique_ptr<ast::Expression>>>
    ArrayItems(std::vector<std::unique_ptr<ast::Expression>> init = {});

    boost::optional<std::vector<ast::MapLit::MapItem>>
    MapItems(std::vector<ast::MapLit::MapItem> init = {});

    std::unique_ptr<ast::Literal> NonEmptyMapArray();

    std::unique_ptr<ast::MapLit>
    ConfirmedMapLiteral(std::unique_ptr<ast::Expression> first_key);

    std::unique_ptr<ast::ArrayLit>
    ConfirmedArrayLiteral(std::unique_ptr<ast::Expression> first_elem);

    std::unique_ptr<ast::Literal> MapArrayLiteral();

    std::unique_ptr<ast::TypeExpr> TypeInParen(const lexer::Token &);

    boost::optional<type::BuiltInAtom::Type> AssertSimpleType();

    std::unique_ptr<ast::MapType> MapType(const lexer::Token &);

    std::unique_ptr<ast::ArrayType> ArrayType(const lexer::Token &);

    std::unique_ptr<ast::FunctionType> FunctionType(const lexer::Token &);

    std::unique_ptr<ast::TypeExpr> TypeExpression();

    std::unique_ptr<ast::Literal> Literal();

    std::unique_ptr<ast::Expression> InterExpr();

    bool InterpolateAction(
        lexer::Token, bool exit_on_err, ast::InterpolatedString &, bool &err);

    boost::optional<ast::InterpolatedString> Interpolatable(bool exit_on_err);

    std::unique_ptr<ast::StringLit> StringLiteral();

    std::unique_ptr<ast::Literal> PathLiteral();

    nullptr_t Recover(std::vector<lexer::Token::Type> stops);

    nullptr_t Recover(std::function<bool(lexer::Token::Type)>);

    nullptr_t RecoverFromStatement();

    nullptr_t RecoverFromExpression(std::vector<lexer::Token::Type> = {});

    bool OptionalStatementEnd();

    bool AssertStatementEnd();

    void SkipStatementEnd();

    boost::optional<lexer::Token::Type> SkipToken();

    template <typename T>
    T WithRecovery(
        T (Parser::*parse)(), std::vector<lexer::Token::Type> nexts,
        bool skip_space = true, bool final_record = true) {
        using util::Has;
        auto recover_stops = nexts;
        recover_stops.push_back(lexer::Token::Type::kLineBreak);
        auto expr = (this->*parse)();
        if (not expr)
            Recover(recover_stops);
        else if (auto l = detail::Lookahead(s_.lexer, skip_space)) {
            if (Has(nexts, l->type)) {
                return expr;
            } else {
                s_.ExpectToken(nexts.front());
                Recover(recover_stops);
            }
        }
        auto l = detail::LookaheadAsToken(s_.lexer, skip_space);
        if (final_record and not Has(nexts, l.type))
            s_.ExpectToken(nexts.front());
        return nullptr;
    }

  private:
    detail::ParserState s_;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_PARSER_H_