#include "sushi/code-generation/expr-visitor.h"
#include "sushi/code-generation/util.h"

namespace sushi {
namespace code_generation {

#define EXPR_VISITING_IMPL(T, t) void ExprVisitor::Visit(const T &t)

using ST = TypeVisitor::SimplifiedType;

EXPR_VISITING_IMPL(ast::Variable, variable) {
    auto new_name = scope_manager->FindNewName(variable.var.name, scope);
    if (is_left_value) {
        val = new_name;
    } else {
        auto type = environment.LookUp(&variable);
        TypeVisitor type_visitor;
        type->AcceptVisitor(type_visitor);
        switch (type_visitor.type) {
        default: assert(false && "Type is not supposed to be here"); break;
        case ST::kInt:
        case ST::kBool:
        case ST::kUnit:
        case ST::kFd:
        case ST::kExitCode:
        case ST::kPath:
        case ST::kRelPath:
        case ST::kString:
        case ST::kChar:
        case ST::kFunc: val = "${" + new_name + '}'; break;
        case ST::kArray:
            val = (boost::format("\"${%1%[@]}\"") % new_name).str();
            break;
        case ST::kMap:
            auto temp = scope_manager->GetNewTemp();
            code_before =
                (boost::format(kMapVarCodeBeforeTemplate) % temp % new_name)
                    .str();
            val = (boost::format(R"("$%1%")") % temp).str();
            new_ids.insert(temp);
            break;
        }
    }
    raw_id = new_name;
}
EXPR_VISITING_IMPL(ast::Literal, literal) {
    LiteralVisitor literal_visitor(scope_manager, environment, scope);
    literal.AcceptVisitor(literal_visitor);
    code_before = literal_visitor.code_before;
    val = literal_visitor.val;
    raw_id = literal_visitor.raw_id;
}
EXPR_VISITING_IMPL(ast::UnaryExpr, unary_expr) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    ExprVisitor expr_visitor(scope_manager, environment, scope);
    unary_expr.expr->AcceptVisitor(expr_visitor);
    // new_ids.merge(expr_visitor.new_ids);
    MergeSets(new_ids, expr_visitor.new_ids);

    code_before = expr_visitor.code_before + '\n';
    using UOP = ast::UnaryExpr::Operator;
    switch (unary_expr.op) {
    case UOP::kNot:
        code_before +=
            (boost::format("%1%=$((! %2%))") % temp_name % expr_visitor.val)
                .str();
        val = "${" + temp_name + '}';
        break;
    case UOP::kNeg:
        code_before +=
            (boost::format("%1%=$((- %2%))") % temp_name % expr_visitor.val)
                .str();
        val = "${" + temp_name + '}';
        break;
    case UOP::kPos:
        code_before += (boost::format("%1%=`_sushi_abs_ %2%`") % temp_name %
                        expr_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    }
}
EXPR_VISITING_IMPL(ast::BinaryExpr, binary_expr) {
    ExprVisitor lhs_visitor(scope_manager, environment, scope);
    ExprVisitor rhs_visitor(scope_manager, environment, scope);
    binary_expr.lhs->AcceptVisitor(lhs_visitor);
    binary_expr.rhs->AcceptVisitor(rhs_visitor);

    // new_ids.merge(lhs_visitor.new_ids);
    MergeSets(new_ids, lhs_visitor.new_ids);
    // new_ids.merge(rhs_visitor.new_ids);
    MergeSets(new_ids, rhs_visitor.new_ids);

    code_before +=
        lhs_visitor.code_before + '\n' + rhs_visitor.code_before + '\n';

    // Get whole expression's type or (lhs or rhs)'s type?
    auto type = environment.LookUp(&binary_expr);
    TypeVisitor type_visitor;
    type->AcceptVisitor(type_visitor);

    using BOP = ast::BinaryExpr::Operator;
// Use macro to make code short...
#define TRANSLATE_OP(op)                                                       \
    Translate##op(lhs_visitor, rhs_visitor, type_visitor.type)

    switch (binary_expr.op) {
    case BOP::kAdd: TRANSLATE_OP(Add); break;
    case BOP::kMinus: TRANSLATE_OP(Minus); break;
    case BOP::kMult: TRANSLATE_OP(Mult); break;
    case BOP::kDiv: TRANSLATE_OP(Div); break;
    case BOP::kMod: TRANSLATE_OP(Mod); break;
    case BOP::kLess: TRANSLATE_OP(Less); break;
    case BOP::kGreat: TRANSLATE_OP(Great); break;
    case BOP::kLessEq: TRANSLATE_OP(LessEq); break;
    case BOP::kGreatEq: TRANSLATE_OP(GreatEq); break;
    case BOP::kEqual: TRANSLATE_OP(Equal); break;
    case BOP::kNotEq: TRANSLATE_OP(NotEq); break;
    case BOP::kAnd: TRANSLATE_OP(And); break;
    case BOP::kOr: TRANSLATE_OP(Or); break;
    }

#undef TRANSLATE_OP
}
EXPR_VISITING_IMPL(ast::CommandLike, cmd_like) {
    CmdLikeVisitor cmdlike_visitor(scope_manager, environment, scope);
    cmd_like.AcceptVisitor(cmdlike_visitor);
    code_before = cmdlike_visitor.code_before;
    val = cmdlike_visitor.val;
    raw_id = cmdlike_visitor.raw_id;
}
EXPR_VISITING_IMPL(ast::Indexing, indexing) {
    ExprVisitor indexable_visitor(scope_manager, environment, scope, true);
    ExprVisitor index_visitor(scope_manager, environment, scope, false);
    indexing.indexable->AcceptVisitor(indexable_visitor);
    indexing.index->AcceptVisitor(index_visitor);

    code_before +=
        indexable_visitor.code_before + '\n' + index_visitor.code_before;
    raw_id = indexable_visitor.val + '[' + index_visitor.val + ']';
    val = "${" + raw_id + '}';
}

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
        code_before += (boost::format(kIntAddTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kString:
        code_before += (boost::format(kStringConcatTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kArray:
        code_before += (boost::format(kArrayConcatTempplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = (boost::format(kArrayValTemplate) % temp_name).str();
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

constexpr char kIntMinusTemplate[] = "local %1%=$((%2% - %3%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Minus) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntMinusTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
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
        code_before += (boost::format(kIntMultTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kString:
        code_before += (boost::format(kIntMultTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

constexpr char kIntDivTemplate[] = "local %1%=$((%2% / %3%))";
constexpr char kPathConcatTemplate[] =
    "local %1%=`_sushi_path_concat_ %2% %3%`";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Div) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntDivTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kPath:
    case ST::kRelPath:
        code_before += (boost::format(kPathConcatTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

constexpr char kIntModTemplate[] = "local %1%=$((%2% %% %3%))";

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Mod) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntModTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
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
        code_before += (boost::format(kIntCompTemplate) % temp_name %
                        lhs_visitor.val % "<" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name %
                        lhs_visitor.val % "<" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Great) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name %
                        lhs_visitor.val % ">" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name %
                        lhs_visitor.val % ">" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, LessEq) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name %
                        lhs_visitor.val % "<=" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name %
                        lhs_visitor.val % "<=" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
    }
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, GreatEq) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;
    switch (type) {
    case ST::kInt:
        code_before += (boost::format(kIntCompTemplate) % temp_name %
                        lhs_visitor.val % ">=" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kChar:
    case ST::kString:
        code_before += (boost::format(kStringCompTemplate) % temp_name %
                        lhs_visitor.val % ">=" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
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
        code_before += (boost::format(kStringCompTemplate) % temp_name %
                        lhs_visitor.val % "==" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kBool:
    case ST::kInt:
    case ST::kExitCode:
    case ST::kFd:
        code_before += (boost::format(kIntCompTemplate) % temp_name %
                        lhs_visitor.val % "==" % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kPath:
    case ST::kRelPath:
        code_before += (boost::format(kPathEqTemplate) % temp_name %
                        lhs_visitor.val % rhs_visitor.val)
                           .str();
        val = "${" + temp_name + '}';
        break;
    case ST::kArray:
        code_before += (boost::format(kArrayEqTemplate) % temp_name %
                        lhs_visitor.raw_id % rhs_visitor.raw_id)
                           .str();
        val = "${" + temp_name + '}';
        break;
    default: assert(false && "Type is not supposed to be here");
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

    code_before += (boost::format(kBoolAndOrTemplate) % temp_name %
                    lhs_visitor.val % "&&" % rhs_visitor.val)
                       .str();
    val = "${" + temp_name + '}';
}

EXPR_VISITOR_TRANSLATE_IMPL(ExprVisitor, Or) {
    auto temp_name = scope_manager->GetNewTemp();
    new_ids.insert(temp_name);
    raw_id = temp_name;

    code_before += (boost::format(kBoolAndOrTemplate) % temp_name %
                    lhs_visitor.val % "||" % rhs_visitor.val)
                       .str();
    val = "${" + temp_name + '}';
}

} // namespace code_generation
} // namespace sushi
