#ifndef SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
#define SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_

#include "./expression.h"
#include "boost/variant.hpp"
#include "sushi/ast/interpolated-string.h"
#include <string>
#include <vector>

namespace sushi {

struct Redirection {
    enum class Type { kIn = 1, kOut = 2 };

    // TODO: FdLiteral internal_;
    Redirection::Type redir_type;
    // if redir_type_ == kOut, external_ can be nullptr, which means "here"
    std::unique_ptr<Expression> external_;
    // if redir_type_ == kIn, this field is redundant
    bool append;
};

class FunctionCall;
class Command;

using CommandLikeVisitor = sushi::util::Visitor<FunctionCall, Command>;

class CommandLike : public Expression {
  public:
    SUSHI_ACCEPT_VISITOR(Expression)
    SUSHI_VISITABLE(CommandLikeVisitor)

    CommandLike(std::vector<Redirection> redirs) : redirs_(std::move(redirs)) {}

  private:
    std::vector<Redirection> redirs_;
};

class FunctionCall : public CommandLike {
  public:
    SUSHI_ACCEPT_VISITOR(CommandLike)

  private:
    std::string func_name_;
    std::vector<std::unique_ptr<Expression>> paramters_;
};

class Command : public CommandLike {
  public:
    SUSHI_ACCEPT_VISITOR(CommandLike)

    using CommandParam =
        boost::variant<InterpolatedString, std::unique_ptr<Expression>>;

  private:
    std::string cmd_name_;
    std::vector<CommandParam> parameters_;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_COMMAND_LIKE_H_
