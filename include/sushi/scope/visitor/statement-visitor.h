#ifndef SUSHI_SCOPE_STATEMENT_VISITOR_H_
#define SUSHI_SCOPE_STATEMENT_VISITOR_H_

#include "expression-visitor.h"
#include "sushi/ast.h"
#include "sushi/scope/environment.h"

namespace sushi {
namespace scope {

struct StatementVisitor : public ast::StatementVisitor::Const {
    Environment &environment;
    std::shared_ptr<Scope> &scope;

    StatementVisitor(Environment &environment, std::shared_ptr<Scope> &scope)
        : environment(environment), scope(scope){};

    SUSHI_VISITING(ast::Assignment, assignment) {
        // visit sub
        ExpressionVisitor expression_visitor(environment, scope);
        assignment.lvalue->AcceptVisitor(expression_visitor);
        assignment.value->AcceptVisitor(expression_visitor);
    }

    SUSHI_VISITING(ast::VariableDef, var_def) {
        // visit sub
        ExpressionVisitor expression_visitor(environment, scope);
        var_def.value->AcceptVisitor(expression_visitor);
        // insert info
        Scope::IdentInfo info = {var_def.start_location.get(), scope.get()};
        scope->Insert(var_def.name, info);
    }

    SUSHI_VISITING(ast::FunctionDef, func_def) {
        // visit sub
        std::shared_ptr<Scope> sub_scope(new Scope(scope));
        environment.Insert(&func_def.body, sub_scope);
        StatementVisitor visitor(environment, sub_scope);
        for (auto &statement : func_def.body.statements) {
            // visitor.visit(Type statement)
            statement->AcceptVisitor(visitor);
        }
        // insert info
        Scope::IdentInfo info = {func_def.start_location.get(), scope.get()};
        scope->Insert(func_def.name, info);
    }

    SUSHI_VISITING(ast::IfStmt, if_stmt) {
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

    SUSHI_VISITING(ast::ReturnStmt, return_stmt) {
        // visit sub
        ExpressionVisitor expression_visitor(environment, scope);
        return_stmt.value->AcceptVisitor(expression_visitor);
    }

    SUSHI_VISITING(ast::SwitchStmt, switch_stmt) {
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

    SUSHI_VISITING(ast::ForStmt, for_stmt) {
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
        Scope::IdentInfo info = {for_stmt.start_location.get(), scope.get()};
        scope->Insert(condition.ident_name, info);
    }

    SUSHI_VISITING(ast::LoopControlStmt, loop_control_stmt) {
        // pass
    }

    SUSHI_VISITING(ast::Expression, expression) {
        ExpressionVisitor expression_visitor(environment, scope);
        expression.AcceptVisitor(expression_visitor);
    }
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_STATEMENT_VISITOR_H_
