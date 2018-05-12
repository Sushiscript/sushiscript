#ifndef SUSHI_PARSER_PARSER_H_
#define SUSHI_PARSER_PARSER_H_

#include "sushi/ast.h"
#include "sushi/lexer.h"
#include "sushi/parser/detail/parser-state.h"
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
    Parser(lexer::Lexer lexer) : s_(std::move(lexer)) {}

    ParsingResult Parse() {
        SUSHI_PARSER_NEW_BLOCK(s_, 0);
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

    ast::Program Program();

    ast::Program Block();

    int DetermineBlockIndent();

    boost::optional<std::unique_ptr<ast::Statement>> CurrentBlockStatement();

    std::unique_ptr<ast::Statement> Statement();

    std::unique_ptr<ast::Statement> Definition();

    std::unique_ptr<ast::ReturnStmt> Return();

    std::unique_ptr<ast::IfStmt> If();

    std::unique_ptr<ast::ForStmt> For();

    std::unique_ptr<ast::SwitchStmt> Switch();

    std::unique_ptr<ast::LoopControlStmt> Break();

    std::unique_ptr<ast::LoopControlStmt> Continue();

    std::unique_ptr<ast::Statement> ExpressionOrAssignment();

    std::unique_ptr<ast::Expression> Expression();

    std::unique_ptr<ast::Expression>
    PrecedenceClimb(std::unique_ptr<ast::Expression> lhs, int min_prec);

    std::unique_ptr<ast::Expression> StartWithIdentifier();

    std::unique_ptr<ast::FunctionCall> FunctionCall();

    std::unique_ptr<ast::Expression> UnaryOperation();

    std::unique_ptr<ast::Indexing> Index();

    std::unique_ptr<ast::Expression> AtomExpr();

    std::unique_ptr<ast::Command> Command();

    std::unique_ptr<ast::Literal> MapArrayLiteral();

    std::unique_ptr<ast::TypeExpr> TypeExpression();

    std::unique_ptr<ast::Literal> Literal();

    std::unique_ptr<ast::Expression> InterExpr();

    bool InterpolateAction(
        lexer::Token, bool exit_on_err, ast::InterpolatedString &, bool &err);

    boost::optional<ast::InterpolatedString> Interpolatable(bool exit_on_err);

    std::unique_ptr<ast::StringLit> StringLiteral();

    std::unique_ptr<ast::Literal> PathLiteral();

    nullptr_t Recover(std::vector<lexer::Token::Type> stops);

    nullptr_t Recover(bool (*)(lexer::Token::Type));

    boost::optional<lexer::Token::Type> SkipToken();

  private:
    detail::ParserState s_;
};

} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_PARSER_H_