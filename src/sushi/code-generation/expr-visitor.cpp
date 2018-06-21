#include "sushi/code-generation/expr-visitor.h"

namespace sushi {
namespace code_generation {

using ST = TypeVisitor::SimplifiedType;

constexpr char kArrayValTemplate[] = "${%1%[@]}";

constexpr char kIntAddTemplate[] = "local %1%=$((%2% + %3%))";
constexpr char kStringConcatTemplate[] = R"(local %1%="%2%%3%")";
constexpr char kArrayConcatTempplate[] = "local %1%=(%2% %3%)";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Add) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntAddTemplate) % temp_name
                                                       % lhs_visitor.val
                                                       % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kString:
        code_before += (boost::format(kStringConcatTemplate) % temp_name
                                                             % lhs_visitor.val
                                                             % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kArray:
        code_before += (boost::format(kArrayConcatTempplate) % temp_name
                                                             % lhs_visitor.val
                                                             % rhs_visitor.val).str();
        val = (boost::format(kArrayValTemplate) % temp_name).str();
        break;
    }
}

constexpr char kIntMinusTemplate[] = "local %1%=$((%2% - %3%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Minus) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntMinusTemplate) % temp_name
                                                         % lhs_visitor.val
                                                         % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

constexpr char kIntMultTemplate[] = "local %1%=$((%2% * %3%))";
constexpr char kStringDupTemplate[] = "local %1%=`_sushi_dup_str_ %2% %3%`";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Mult) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntMultTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kString:
        code_before += (boost::format(kIntMultTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

constexpr char kIntDivTemplate[] = "local %1%=$((%2% / %3%))";
constexpr char kPathConcatTemplate[] = "local %1%=`_sushi_path_concat_ %2% %3%`";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Div) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntDivTemplate) % temp_name
                                                       % lhs_visitor.val
                                                       % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kPath:
    case ST::kRelPath:
        code_before += (boost::format(kPathConcatTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

constexpr char kIntModTemplate[] = "local %1%=$((%2% %% %3%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Mod) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntModTemplate) % temp_name
                                                       % lhs_visitor.val
                                                       % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

constexpr char kStringCompTemplate[] = "[[ %2% %3% %4% ]]; %1%=$((1 - $?))";
constexpr char kIntCompTemplate[] = "%1%=$((%2% %3% %4%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Less) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % "<"
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % "<"
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Great) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % ">"
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % ">"
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, LessEq) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % "<="
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % "<="
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, GreatEq) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % ">="
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % ">="
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    }
}

constexpr char kPathEqTemplate[] = "local %1%=`_sushi_file_eq_ %2% %3%`";
constexpr char kArrayEqTemplate[] = "local %1%=`_sushi_compare_array_ %2% %3%`";

constexpr char kAssignTemplate[] = "local %1%=%2%";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Equal) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kUnit:
        code_before += (boost::format(kAssignTemplate) % temp_name % "1").str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name
                                                           % lhs_visitor.val
                                                           % "=="
                                                           % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kBool:
    case ST::kInt:
    case ST::kExitCode:
    case ST::kFd:
        code_before += (boost::format(kIntCompTemplate) % temp_name
                                                        % lhs_visitor.val
                                                        % "=="
                                                        % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kPath:
    case ST::kRelPath:
        code_before += (boost::format(kPathEqTemplate) % temp_name
                                                       % lhs_visitor.val
                                                       % rhs_visitor.val).str();
        val = "${" + temp_name + '}';
        break;
    case ST::kArray:
        code_before += (boost::format(kArrayEqTemplate) % temp_name
                                                        % lhs_visitor.raw_id
                                                        % rhs_visitor.raw_id).str();
        val = "${" + temp_name + '}';
        break;
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, NotEq) {
    TranslateEqual(lhs_visitor, rhs_visitor, type);
    code_before += (boost::format("%1%=$((1 - %2%))") % raw_id % val).str();
}

constexpr char kBoolAndOrTemplate[] = "%1%=$((%2% %3% %4%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, And) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    code_before += (boost::format(kBoolAndOrTemplate) % temp_name
                                                      % lhs_visitor.val
                                                      % "&&"
                                                      % rhs_visitor.val).str();
    val = "${" + temp_name + '}';
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Or) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    code_before += (boost::format(kBoolAndOrTemplate) % temp_name
                                                      % lhs_visitor.val
                                                      % "||"
                                                      % rhs_visitor.val).str();
    val = "${" + temp_name + '}';
}

} // namespace sushi
} // namespace code_generation
