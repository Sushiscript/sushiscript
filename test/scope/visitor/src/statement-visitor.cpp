#include "sushi/scope.h"
#include "../include/statement-visitor.h"

namespace sushi {
namespace scope {
namespace test {

#define VISIT(T, t) void StatementVisitor::Visit(const T &t)

VISIT(ast::Assignment, assignment) {
    ExpressionVisitor expression_visitor;
    assignment.lvalue->AcceptVisitor(expression_visitor);
    assignment.value->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::VariableDef, var_def) {
    ExpressionVisitor expression_visitor;
    var_def.value->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::FunctionDef, func_def) {
    // body
    programs.push_back(&func_def.body);
    StatementVisitor visitor;
    for (auto &statement : func_def.body.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor);
    }
    MergeVector(identifiers, visitor.identifiers);
    MergeVector(programs, visitor.programs);
}

VISIT(ast::IfStmt, if_stmt) {
    ExpressionVisitor expression_visitor;
    if_stmt.condition->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
    // true body
    StatementVisitor visitor_true;
    for (auto &statement : if_stmt.true_body.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor_true);
    }
    MergeVector(identifiers, visitor_true.identifiers);
    programs.push_back(&if_stmt.true_body);
    MergeVector(programs, visitor_true.programs);

    // false body
    if (!if_stmt.false_body.statements.empty()) {
        StatementVisitor visitor_false;
        for (auto &statement : if_stmt.false_body.statements) {
            // visitor.visit(Type statement)
            statement->AcceptVisitor(visitor_false);
        }
        MergeVector(identifiers, visitor_false.identifiers);
        programs.push_back(&if_stmt.false_body);
        MergeVector(programs, visitor_false.programs);
    }
}

VISIT(ast::ReturnStmt, return_stmt) {
    ExpressionVisitor expression_visitor;
    return_stmt.value->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::SwitchStmt, switch_stmt) {
    // visit sub
    // switched key
    ExpressionVisitor expression_visitor;
    switch_stmt.switched->AcceptVisitor(expression_visitor);
    // switch case
    for (auto &each_case : switch_stmt.cases) {
        // condition
        if (each_case.condition)
            each_case.condition->AcceptVisitor(expression_visitor);
        // body
        StatementVisitor visitor;
        for (auto &statement : each_case.body.statements) {
            // visitor.visit(Type statement)
            statement->AcceptVisitor(visitor);
        }
        MergeVector(identifiers, visitor.identifiers);
        programs.push_back(&each_case.body);
        MergeVector(programs, visitor.programs);
    }
    MergeVector(identifiers, expression_visitor.identifiers);
}

VISIT(ast::ForStmt, for_stmt) {
    // condition
    auto &condition = for_stmt.condition;
    ExpressionVisitor expression_visitor;
    condition.condition->AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
    // body
    StatementVisitor visitor;
    for (auto &statement : for_stmt.body.statements) {
        statement->AcceptVisitor(visitor);
    }
    MergeVector(identifiers, visitor.identifiers);
    programs.push_back(&for_stmt.body);
    MergeVector(programs, visitor.programs);
}

VISIT(ast::LoopControlStmt, loop_control_stmt) {
    // pass
}

VISIT(ast::Expression, expression) {
    ExpressionVisitor expression_visitor;
    expression.AcceptVisitor(expression_visitor);
    MergeVector(identifiers, expression_visitor.identifiers);
}

}
} // namespace scope
} // namespace sushi
