#ifndef SUSHI_CODE_GEN_LITERAL_VISITOR_H_
#define SUSHI_CODE_GEN_LITERAL_VISITOR_H_

#include "sushi/ast.h"

#include <set>

namespace sushi {
namespace code_generation {

struct LiteralVisitor : public ast::LiteralVisitor::Const {
    std::string val;
    std::string code_before;
    std::string raw_id;
    std::set<std::string> new_ids;

    LiteralVisitor(
        std::shared_ptr<ScopeManager> scope_manager,
        const scope::Environment & environment,
        const scope::Scope * scope
        ) : scope_manager(scope_manager),
            environment(environment),
            scope(scope) {}

    SUSHI_VISITING(ast::IntLit, int_lit) {
        val = "$((" + std::to_string(int_lit.value) + "))";
    }
    SUSHI_VISITING(ast::CharLit, char_lit) {
        val = '"' + char_lit.value + '"';
    }
    SUSHI_VISITING(ast::BoolLit, bool_lit) {
        if (bool_lit.value) {
            val = '$((1))';
        } else {
            val = '$((0))';
        }
    }
    SUSHI_VISITING(ast::UnitLit, unit_lit) {
        val = "0";
    }
    SUSHI_VISITING(ast::FdLit, fd_lit) {
        using V = ast::FdLit::Value;
        switch (fd_lit.value) {
        case V::kStdin: val = "0"; break;
        case V::kStdout: val = "1"; break;
        case V::kStderr: val = "2"; break;
        }
    }

    SUSHI_VISITING(ast::StringLit, string_lit) {
        TranslateInterpolation(string_lit.value);
    }
    SUSHI_VISITING(ast::PathLit, path_lit) {
        TranslateInterpolation(path_lit.value);
    }
    SUSHI_VISITING(ast::RelPathLit, relPath_lit) {
        TranslateInterpolation(relPath_lit.value);
    }

    SUSHI_VISITING(ast::ArrayLit, array_lit) {
        auto temp_name = scope_manager->GetNewTemp();
        new_ids.insert(temp_name);
        raw_id = temp_name;

        std::string lit_inside;

        constexpr char kArrayLitCodeBeforeTemplate[] = "local %1%=(%2%)";

        bool is_first = false;
        for (auto &i : array_lit.value) {
            ExprVisitor expr_visitor(scope_manager, environment, scope);
            i->AcceptVisitor(expr_visitor);
            code_before += expr_visitor.code_before + '\n';
            new_ids.merge(expr_visitor.new_ids);
            if (is_first) {
                is_first = false;
                lit_inside += expr_visitor.val;
            } else {
                lit_inside += ' ' + expr_visitor.val;
            }
        }

        code_before += (boost::format(kArrayLitCodeBeforeTemplate) % temp_name % lit_inside).str();

        val = "${" + temp_name + '}';
    }

    SUSHI_VISITING(ast::MapLit, map_lit) {
        auto temp_name = scope_manager->GetNewTemp();
        new_ids.insert(temp_name);
        raw_id = temp_name;

        std::string lit_inside;

        constexpr char kMapLitCodeBeforeTemplate[] = "local %1%=(%2%)";
        constexpr char kMapItemTemplate[] = "[%1%]=%2%";

        bool is_first = false;
        for (auto &i : map_lit.value) {
            ExprVisitor key_visitor(scope_manager, environment, scope);
            ExprVisitor val_visitor(scope_manager, environment, scope);
            i.first->AcceptVisitor(key_visitor);
            i.first->AcceptVisitor(val_visitor);
            code_before += key_visitor.code_before + '\n' + val_visitor.code_before + '\n';
            new_ids.merge(key_visitor.new_ids);
            new_ids.merge(val_visitor.new_ids);
            if (is_first) {
                is_first = false;
                lit_inside += (boost::format(kMapItemTemplate) % key_visitor.val % val_visitor.val).str();
            } else {
                lit_inside += ' ' + (boost::format(kMapItemTemplate) % key_visitor.val % val_visitor.val).str();
            }
        }

        code_before += (boost::format(kMapLitCodeBeforeTemplate) % temp_name % lit_inside).str();

        val = "${" + temp_name + '}';
    }

    std::shared_ptr<ScopeManager> scope_manager;
    const scope::Environment & environment;
    const scope::Scope * scope;

    void TranslateInterpolation(const ast::InterpolatedString &inter_str) {
        std::string lit_str;
        inter_str.Traverse([this, &lit_str](const std::string &str) {
            lit_str += str;
        }, [this, &lit_str](const ast::Expression & expr) {
            auto temp_name = scope_manager->GetNewTemp();
            new_ids.insert(temp_name);
            ExprVisitor expr_visitor(scope_manager, environment, scope);
            expr.AcceptVisitor(expr_visitor);
            new_ids.merge(expr_visitor.new_ids);

            code_before += expr_visitor.code_before + '\n';
            code_before += (boost::format("local %1%=%2%") % temp_name % expr_visitor.val).str();

            lit_str += "${" + temp_name + '}';
        });
        val = (boost::format("\"%1%\"") % lit_str).str();
    }
};

} // namespace code_generation
} // namespace sushi

#endif
