#ifndef SUSHI_AST_STATEMENT_CONCRETE_STMT_H_
#define SUSHI_AST_STATEMENT_CONCRETE_STMT_H_

#include "./statement.h"
#include "sushi/ast/expression.h"
#include "sushi/ast/type-expr.h"
#include "sushi/util/visitor.h"
#include <memory>
#include <vector>

namespace sushi {
namespace ast {

struct Assignment : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    Assignment(
        Identifier ident, std::unique_ptr<Expression> index,
        std::unique_ptr<Expression> value)
        : ident(std::move(ident)), index(std::move(index)),
          value(std::move(value)) {}

    Identifier ident;
    // index is nullptr in a normal assignment
    std::unique_ptr<Expression> index;
    std::unique_ptr<Expression> value;
};

struct VariableDef : Statement {
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

struct FunctionDef : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    struct Parameter {
        std::string name;
        std::unique_ptr<TypeExpr> type;
    };

    FunctionDef(
        bool is_export, const std::string &name, std::vector<Parameter> params,
        std::unique_ptr<TypeExpr> ret_type, Program body)
        : is_export(is_export), name(name), params(std::move(params)),
          ret_type(std::move(ret_type)), body(std::move(body)) {}

    bool is_export;
    std::string name;
    std::vector<Parameter> params;
    // ret_type can be nullptr
    std::unique_ptr<TypeExpr> ret_type;
    Program body;
};

struct IfStmt : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    IfStmt(
        std::unique_ptr<Expression> condition, Program true_body,
        Program false_body)
        : condition(std::move(condition)), true_body(std::move(true_body)),
          false_body(std::move(false_body)) {}

    std::unique_ptr<Expression> condition;
    Program true_body;
    // false_body_ can be empty
    Program false_body;
};

struct ReturnStmt : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    ReturnStmt(std::unique_ptr<Expression> value) : value(std::move(value)) {}

    // value can be empty, default to be "()"(unit)
    std::unique_ptr<Expression> value;
};

struct SwitchStmt : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    struct Case {
        // nullptr means default case
        std::unique_ptr<Expression> condition;
        Program body;
    };

    SwitchStmt(std::vector<Case> cases) : cases(std::move(cases)) {}

    std::vector<Case> cases;
};

struct ForStmt : Statement {
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

    ForStmt(Condition condition, Program body)
        : condition(std::move(condition)), body(std::move(body)) {}

    Condition condition;
    Program body;
};

struct LoopControlStmt : Statement {
    SUSHI_ACCEPT_VISITOR_FROM(Statement)

    enum struct Value : uint8_t { kBreak = 1, kContinue = 2 };

    LoopControlStmt(LoopControlStmt::Value control_type, int level)
        : control_type(control_type), level(level) {}

    LoopControlStmt::Value control_type;
    int level;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_STATEMENT_CONCRETE_STMT_H_
