#ifndef SUSHI_AST_STATEMENT_STATEMENT_H_
#define SUSHI_AST_STATEMENT_STATEMENT_H_

#include "sushi/util/visitor.h"
#include "boost/optional.hpp"
#include "sushi/lexer/token-location.h"
#include <memory>
#include <vector>

namespace sushi {
namespace ast {

struct VariableDef;
struct FunctionDef;
struct IfStmt;
struct ReturnStmt;
struct SwitchStmt;
struct ForStmt;
struct LoopControlStmt;
struct Expression;

using StatementVisitor = sushi::util::DefineVisitor<
    VariableDef, FunctionDef, IfStmt, ReturnStmt, SwitchStmt, ForStmt,
    LoopControlStmt, Expression>;

struct Statement {
    SUSHI_VISITABLE(StatementVisitor)
    virtual ~Statement() {}

    boost::optional<TokenLocation> start_location;
};

struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_STATEMENT_STATEMENT_H_
