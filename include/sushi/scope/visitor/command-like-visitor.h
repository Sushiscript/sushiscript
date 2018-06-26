#ifndef SUSHI_SCOPE_COMMAND_LIKE_VISITOR_H_
#define SUSHI_SCOPE_COMMAND_LIKE_VISITOR_H_

#include "sushi/ast.h"

namespace sushi {
namespace scope {

struct CommandLikeVisitor : public ast::CommandLikeVisitor::Const {
    Environment &environment;
    std::shared_ptr<Scope> &scope;

    CommandLikeVisitor(Environment &environment, std::shared_ptr<Scope> &scope)
        : environment(environment), scope(scope){};

    SUSHI_VISITING(ast::FunctionCall, func_call);
    SUSHI_VISITING(ast::Command, command);
};

} // namespace scope
} // namespace sushi

#endif
