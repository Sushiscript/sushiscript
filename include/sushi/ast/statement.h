#ifndef SUSHI_AST_STATEMENT_H_
#define SUSHI_AST_STATEMENT_H_

#include "./expression.h"
#include "./type-expr.h"
#include "sushi/util/visitor.h"
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace sushi {

struct VariableDef;
struct FunctionDef;
struct IfStmt;
struct ReturnStmt;
struct SwitchStmt;
struct ForStmt;
struct LoopControlStmt;

using StatementVisitor = sushi::util::DefineVisitor<
    VariableDef, FunctionDef, IfStmt, ReturnStmt, SwitchStmt, ForStmt,
    LoopControlStmt>;

struct Statement {
    SUSHI_VISITABLE(StatementVisitor)
    virtual ~Statement() {}
};

struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

struct VariableDef : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    VariableDef(
        bool is_export, const std::string &name, std::unique_ptr<TypeExpr> type,
        std::unique_ptr<Expression> value)
        : is_export(is_export), name(name), type(std::move(type)),
          value(std::move(value)) {}

    bool is_export;
    std::string name;
    // type can be nullptr
    std::unique_ptr<TypeExpr> type;
    std::unique_ptr<Expression> value;
};

struct FunctionDef : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    struct Parameter {
        std::string name;
        std::unique_ptr<TypeExpr> type;
    };

    FunctionDef(
        bool is_export, const std::string &name, std::vector<Parameter> params,
        std::unique_ptr<Program> body)
        : is_export(is_export), name(name), params(std::move(params)),
          body(std::move(body)) {}

    bool is_export;
    std::string name;
    std::vector<Parameter> params;
    std::unique_ptr<Program> body;
};

struct IfStmt : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    IfStmt(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Program> true_body, std::unique_ptr<Program> false_body)
        : condition(std::move(condition)), true_body(std::move(true_body)),
          false_body(std::move(false_body)) {}

    std::unique_ptr<Expression> condition;
    std::unique_ptr<Program> true_body;
    // false_body_ can be nullptr
    std::unique_ptr<Program> false_body;
};

struct ReturnStmt : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    std::unique_ptr<Expression> value;
};

struct SwitchStmt : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    struct Case {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Program> body;
    };

    SwitchStmt(std::vector<Case> cases, std::unique_ptr<Program> default_)
        : cases(std::move(cases)), default_(std::move(default_)) {}

    std::vector<Case> cases;
    // default can be nullptr
    std::unique_ptr<Program> default_;
};

struct ForStmt : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    struct Condition {
        bool IsRange() const {
            return not ident_name.empty();
        }

        Condition(
            const std::string &ident_name,
            std::unique_ptr<Expression> condition)
            : ident_name(ident_name), condition(std::move(condition)) {}

        std::string ident_name;
        std::unique_ptr<Expression> condition;
    };

    ForStmt(Condition condition, std::unique_ptr<Program> body)
        : condition(std::move(condition)), body(std::move(body)) {}

    Condition condition;
    std::unique_ptr<Program> body;
};

struct LoopControlStmt : public Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    enum struct Value : uint8_t { kBreak = 1, kContinue = 2 };

    LoopControlStmt(LoopControlStmt::Value control_type, int level)
        : control_type(control_type), level(level) {}

    LoopControlStmt::Value control_type;
    int level;
};

} // namespace sushi

#endif // SUSHI_AST_STATEMENT_H_
