#include "boost/format.hpp"
#include "sushi/code-generation.h"

namespace sushi {
namespace code_generation {

#define LITERAL_VISITING_IMPL(T, t) void LiteralVisitor::Visit(const T &t)

LITERAL_VISITING_IMPL(ast::IntLit, int_lit) {
    auto temp_name = GetTempName();
    code_before = (boost::format("local %1%=$((%2%))") % temp_name %
                   std::to_string(int_lit.value))
                      .str();
    val = "${" + temp_name + '}';
}
LITERAL_VISITING_IMPL(ast::CharLit, char_lit) {
    auto temp_name = GetTempName();
    code_before =
        (boost::format("local %1%=\"%2%\"") % temp_name % char_lit.value).str();
    val = "${" + temp_name + '}';
}
LITERAL_VISITING_IMPL(ast::BoolLit, bool_lit) {
    auto temp_name = GetTempName();
    constexpr char template_[] = "local %1%=$((%2%))";
    if (bool_lit.value) {
        code_before = (boost::format(template_) % temp_name % "1").str();
    } else {
        code_before = (boost::format(template_) % temp_name % "0").str();
    }
    val = "${" + temp_name + '}';
}
LITERAL_VISITING_IMPL(ast::UnitLit, unit_lit) {
    auto temp_name = GetTempName();
    code_before = (boost::format("local %1%=''") % temp_name).str();
    val = "${" + temp_name + '}';
}
LITERAL_VISITING_IMPL(ast::FdLit, fd_lit) {
    auto temp_name = GetTempName();

    std::string fd_str;
    using V = ast::FdLit::Value;
    switch (fd_lit.value) {
    case V::kStdin: fd_str = "0"; break;
    case V::kStdout: fd_str = "1"; break;
    case V::kStderr: fd_str = "2"; break;
    }

    constexpr char template_[] = "local %1%=%2%";
    code_before = (boost::format(template_) % temp_name % fd_str).str();
    val = "${" + temp_name + '}';
}

LITERAL_VISITING_IMPL(ast::StringLit, string_lit) {
    TranslateInterpolation(string_lit.value);
}
LITERAL_VISITING_IMPL(ast::PathLit, path_lit) {
    TranslateInterpolation(path_lit.value);
}
LITERAL_VISITING_IMPL(ast::RelPathLit, relPath_lit) {
    TranslateInterpolation(relPath_lit.value);
}

LITERAL_VISITING_IMPL(ast::ArrayLit, array_lit) {
    auto temp_name = GetTempName();

    std::string lit_inside;

    constexpr char kArrayLitCodeBeforeTemplate[] = "local %1%=(%2%)";

    bool is_first = false;
    for (auto &i : array_lit.value) {
        ExprVisitor expr_visitor(scope_manager, environment, scope);
        i->AcceptVisitor(expr_visitor);
        code_before += expr_visitor.code_before + '\n';
        // new_ids.merge(expr_visitor.new_ids);
        MergeSets(new_ids, expr_visitor.new_ids);
        if (is_first) {
            is_first = false;
            lit_inside += expr_visitor.val;
        } else {
            lit_inside += ' ' + expr_visitor.val;
        }
    }

    code_before +=
        (boost::format(kArrayLitCodeBeforeTemplate) % temp_name % lit_inside)
            .str();
    val = "${" + temp_name + "[@]}";
}

LITERAL_VISITING_IMPL(ast::MapLit, map_lit) {
    auto temp_name = GetTempName();

    std::string lit_inside;

    constexpr char kMapLitCodeBeforeTemplate[] = "local -A %1%=(%2%)";
    constexpr char kMapItemTemplate[] = "[%1%]=%2%";

    bool is_first = false;
    for (auto &i : map_lit.value) {
        ExprVisitor key_visitor(scope_manager, environment, scope);
        ExprVisitor val_visitor(scope_manager, environment, scope);
        i.first->AcceptVisitor(key_visitor);
        i.second->AcceptVisitor(val_visitor);
        code_before +=
            key_visitor.code_before + '\n' + val_visitor.code_before + '\n';
        // new_ids.merge(key_visitor.new_ids);
        MergeSets(new_ids, key_visitor.new_ids);
        // new_ids.merge(val_visitor.new_ids);
        MergeSets(new_ids, val_visitor.new_ids);
        if (is_first) {
            is_first = false;
            lit_inside += (boost::format(kMapItemTemplate) % key_visitor.val %
                           val_visitor.val)
                              .str();
        } else {
            lit_inside += ' ' + (boost::format(kMapItemTemplate) %
                                 key_visitor.val % val_visitor.val)
                                    .str();
        }
    }

    code_before +=
        (boost::format(kMapLitCodeBeforeTemplate) % temp_name % lit_inside)
            .str();
    auto map_lit_inside_temp_name = scope_manager->GetNewTemp();
    new_ids.insert(map_lit_inside_temp_name);
    code_before += '\n' + (boost::format(kMapVarCodeBeforeTemplate) %
                           map_lit_inside_temp_name % temp_name)
                              .str();
    val = "${" + map_lit_inside_temp_name + "}";
}

void LiteralVisitor::TranslateInterpolation(
    const ast::InterpolatedString &inter_str) {
    auto temp_name = GetTempName();
    std::string lit_str;
    inter_str.Traverse(
        [this, &lit_str](const std::string &str) { lit_str += str; },
        [this, &lit_str](const ast::Expression &expr) {
            auto temp_name = scope_manager->GetNewTemp();
            ExprVisitor expr_visitor(scope_manager, environment, scope);
            expr.AcceptVisitor(expr_visitor);
            // new_ids.merge(expr_visitor.new_ids);
            MergeSets(new_ids, expr_visitor.new_ids);

            code_before += expr_visitor.code_before + '\n';
            code_before +=
                (boost::format("local %1%=%2%") % temp_name % expr_visitor.val)
                    .str();

            lit_str += "${" + temp_name + '}';
        });
    code_before +=
        '\n' + (boost::format("local %1%=\"%2%\"") % temp_name % lit_str).str();
    val = "${" + temp_name + '}';
}

} // namespace code_generation
} // namespace sushi
