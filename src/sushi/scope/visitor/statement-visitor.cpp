#include "sushi/scope.h"

namespace sushi {
namespace scope {

#define VISIT(T, t) void StatementVisitor::Visit(const T &t)

VISIT(ast::Assignment, assignment) {
    // visit sub
    ExpressionVisitor expression_visitor(environment, scope);
    assignment.lvalue->AcceptVisitor(expression_visitor);
    assignment.value->AcceptVisitor(expression_visitor);
}

VISIT(ast::VariableDef, var_def) {
    // visit sub
    ExpressionVisitor expression_visitor(environment, scope);
    var_def.value->AcceptVisitor(expression_visitor);
    // insert info
    auto info = Scope::CreateIdentInfo(var_def.start_location, scope.get());
    scope->Insert(var_def.name, info);
}

VISIT(ast::FunctionDef, func_def) {
    // visit sub
    std::shared_ptr<Scope> sub_scope(new Scope(scope));
    environment.Insert(&func_def.body, sub_scope);
    StatementVisitor visitor(environment, sub_scope);
    for (auto &statement : func_def.body.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor);
    }
    // insert info
    auto info = Scope::CreateIdentInfo(func_def.start_location, scope.get());
    scope->Insert(func_def.name, info);
}

VISIT(ast::IfStmt, if_stmt) {
    // visit sub
    // condition
    ExpressionVisitor expression_visitor(environment, scope);
    if_stmt.condition->AcceptVisitor(expression_visitor);
    // true body
    std::shared_ptr<Scope> sub_scope_true(new Scope(scope));
    environment.Insert(&if_stmt.true_body, sub_scope_true);
    StatementVisitor visitor_true(environment, sub_scope_true);
    for (auto &statement : if_stmt.true_body.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor_true);
    }
    // false body
    std::shared_ptr<Scope> sub_scope_false(new Scope(scope));
    environment.Insert(&if_stmt.false_body, sub_scope_false);
    StatementVisitor visitor_false(environment, sub_scope_false);
    for (auto &statement : if_stmt.false_body.statements) {
        // visitor.visit(Type statement)
        statement->AcceptVisitor(visitor_false);
    }
}

VISIT(ast::ReturnStmt, return_stmt) {
    // visit sub
    ExpressionVisitor expression_visitor(environment, scope);
    return_stmt.value->AcceptVisitor(expression_visitor);
}

VISIT(ast::SwitchStmt, switch_stmt) {
    // visit sub
    // switched key
    ExpressionVisitor expression_visitor(environment, scope);
    switch_stmt.switched->AcceptVisitor(expression_visitor);
    // switch case
    for (auto &each_case : switch_stmt.cases) {
        // condition
        each_case.condition->AcceptVisitor(expression_visitor);
        // body
        std::shared_ptr<Scope> sub_scope(new Scope(scope));
        environment.Insert(&each_case.body, sub_scope);
        StatementVisitor visitor(environment, sub_scope);
        for (auto &statement : each_case.body.statements) {
            // visitor.visit(Type statement)
            statement->AcceptVisitor(visitor);
        }
    }
}

VISIT(ast::ForStmt, for_stmt) {
    // visit sub
    // condition
    auto &condition = for_stmt.condition;
    ExpressionVisitor expression_visitor(environment, scope);
    condition.condition->AcceptVisitor(expression_visitor);
    // body
    std::shared_ptr<Scope> sub_scope(new Scope(scope));
    environment.Insert(&for_stmt.body, sub_scope);
    StatementVisitor visitor(environment, sub_scope);
    for (auto &statement : for_stmt.body.statements) {
        statement->AcceptVisitor(visitor);
    }
    // insert info
    auto info = Scope::CreateIdentInfo(for_stmt.start_location, scope.get());
    scope->Insert(condition.ident_name, info);
}

VISIT(ast::LoopControlStmt, loop_control_stmt) {
    // pass
}

VISIT(ast::Expression, expression) {
    ExpressionVisitor expression_visitor(environment, scope);
    expression.AcceptVisitor(expression_visitor);
}

} // namespace scope
} // namespace sushi
