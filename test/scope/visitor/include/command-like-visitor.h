#ifndef SUSHI_SCOPE_TEST_COMMAND_LIKE_VISITOR_H_
#define SUSHI_SCOPE_TEST_COMMAND_LIKE_VISITOR_H_

#include "sushi/scope/environment.h"
#include "sushi/scope/error.h"

namespace sushi {
namespace scope {
namespace test {

struct CommandLikeVisitor : public ast::CommandLikeVisitor::Const {
    CommandLikeVisitor() {}

    SUSHI_VISITING(ast::FunctionCall, func_call);
    SUSHI_VISITING(ast::Command, command);

    std::vector<const ast::Program*> programs;
    std::vector<const ast::Identifier*> identifiers;
};

}
} // namespace scope
} // namespace sushi

#endif
