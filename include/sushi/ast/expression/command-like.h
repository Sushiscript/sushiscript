#ifndef SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
#define SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_

#include "./expression.h"
#include "./identifier.h"
#include "./literal.h"
#include "boost/variant.hpp"
#include "sushi/ast/interpolated-string.h"
#include <string>
#include <vector>

namespace sushi {
namespace ast {

struct Redirection {
    enum struct Direction { kIn = 1, kOut = 2 };

    Redirection(
        FdLit::Value me, Redirection::Direction direction,
        std::unique_ptr<Expression> external, bool append)
        : me(me), direction(direction), external(std::move(external)),
          append(append){};

    FdLit::Value me;

    Redirection::Direction direction;
    // if redir_type_ == kOut, external can be nullptr, which means "here"
    std::unique_ptr<Expression> external;
    // if redir_type_ == kIn, this field is redundant
    bool append;
};

struct FunctionCall;
struct Command;

using CommandLikeVisitor = sushi::util::DefineVisitor<FunctionCall, Command>;

struct CommandLike : Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)
    SUSHI_VISITABLE(CommandLikeVisitor)

    CommandLike(
        std::vector<Redirection> redirs, std::unique_ptr<CommandLike> pipe_next)
        : redirs(std::move(redirs)), pipe_next(std::move(pipe_next)) {}

    std::vector<Redirection> redirs;
    std::unique_ptr<CommandLike> pipe_next;
};

struct FunctionCall : CommandLike {
    SUSHI_ACCEPT_VISITOR_FROM(CommandLike)

    FunctionCall(
        Identifier func, std::vector<std::unique_ptr<Expression>> parameters,
        std::vector<Redirection> redirs, std::unique_ptr<CommandLike> pipe_next)
        : CommandLike(std::move(redirs), std::move(pipe_next)),
          func(std::move(func)), parameters(std::move(parameters)){};

    Identifier func;
    std::vector<std::unique_ptr<Expression>> parameters;
};

struct Command : CommandLike {
    SUSHI_ACCEPT_VISITOR_FROM(CommandLike)

    Command(
        InterpolatedString cmd, std::vector<InterpolatedString> parameters,
        std::vector<Redirection> redirs, std::unique_ptr<CommandLike> pipe_next)
        : CommandLike(std::move(redirs), std::move(pipe_next)),
          cmd(std::move(cmd)), parameters(std::move(parameters)) {}

    InterpolatedString cmd;
    std::vector<InterpolatedString> parameters;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
