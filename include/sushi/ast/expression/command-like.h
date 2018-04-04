#ifndef SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
#define SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_

#include "./expression.h"
#include "./literal.h"
#include "boost/variant.hpp"
#include "sushi/ast/interpolated-string.h"
#include <string>
#include <vector>

namespace sushi {

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

struct CommandLike : public Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)
    SUSHI_VISITABLE(CommandLikeVisitor)

    CommandLike(std::vector<Redirection> redirs) : redirs(std::move(redirs)) {}

    std::vector<Redirection> redirs;
};

struct FunctionCall : public CommandLike {
    SUSHI_ACCEPT_VISITOR_FROM(CommandLike)

    FunctionCall(
        std::string func_name,
        std::vector<std::unique_ptr<Expression>> parameters,
        std::vector<Redirection> redirs)
        : CommandLike(std::move(redirs)), func_name(std::move(func_name)),
          parameters(std::move(parameters)){};

    std::string func_name;
    std::vector<std::unique_ptr<Expression>> parameters;
};

struct Command : public CommandLike {
    SUSHI_ACCEPT_VISITOR_FROM(CommandLike)

    using CommandParam =
        boost::variant<InterpolatedString, std::unique_ptr<Expression>>;

    Command(
        std::string cmd_name, std::vector<CommandParam> parameters,
        std::vector<Redirection> redirs)
        : CommandLike(std::move(redirs)), cmd_name(std::move(cmd_name)),
          parameters(std::move(parameters)) {}

    std::string cmd_name;
    std::vector<CommandParam> parameters;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
