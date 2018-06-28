#include "sushi/type-system/type-check/expression.h"

#define SIMPLE(t) BuiltInAtom::Type::t
#define MAKE_SIMPLE(t) type::BuiltInAtom::Make(SIMPLE(t))

namespace sushi {

namespace type {

namespace {

struct DeduceLiteralVisitor : ast::LiteralVisitor::Const {
    DeduceLiteralVisitor(State& s): s(s) {}

    SUSHI_VISITING(ast::IntLit, i) {
        result = DeduceResult(MAKE_SIMPLE(kInt));
    }
    SUSHI_VISITING(ast::CharLit, c) {
        result = DeduceResult(MAKE_SIMPLE(kChar));
    }
    SUSHI_VISITING(ast::BoolLit, b) {
        result = DeduceResult(MAKE_SIMPLE(kBool));
    }
    SUSHI_VISITING(ast::UnitLit, u) {
        result = DeduceResult(MAKE_SIMPLE(kUnit));
    }
    SUSHI_VISITING(ast::FdLit, f) {
        result = DeduceResult(MAKE_SIMPLE(kFd));
    }
    SUSHI_VISITING(ast::PathLit, p) {
        result = DeduceResult(MAKE_SIMPLE(kPath));
    }
    SUSHI_VISITING(ast::RelPathLit, r) {
        result = DeduceResult(MAKE_SIMPLE(kRelPath));
    }
    SUSHI_VISITING(ast::StringLit, s) {

    }
    SUSHI_VISITING(ast::ArrayLit, a) {

    }
    SUSHI_VISITING(ast::MapLit, m) {

    }

    DeduceResult result;
    State& s;
};

DeduceResult DeduceLiteral(const ast::Literal& l, State& s) {
    DeduceLiteralVisitor v(s);
    l.AcceptVisitor(v);
    return std::move(v.result);
}

struct DeductionVisitor : ast::ExpressionVisitor::Const {
    DeductionVisitor(State &s) : s(s) {}
    SUSHI_VISITING(ast::Variable, var) {
        auto p = s.LookupName(var.var);
        if (not p) result = DeduceResult::Fail();
        result = DeduceResult(std::move(p));
    }
    SUSHI_VISITING(ast::Literal, lit) {
        result = DeduceLiteral(lit, s);
    }
    SUSHI_VISITING(ast::UnaryExpr, u) {}
    SUSHI_VISITING(ast::BinaryExpr, bi) {}
    SUSHI_VISITING(ast::CommandLike, cmd) {}
    SUSHI_VISITING(ast::Indexing, idx) {}

    State &s;
    DeduceResult result;
};

bool ImplicitConvertible(const Type *from, const Type *to) {
    if (from->Equals(to)) return true;
    auto simple_from = from->ToSimple(), simple_to = to->ToSimple();
    if (not(simple_from and simple_to)) return false;
    auto from_type = simple_from->type, to_type = simple_to->type;
    if (from_type == to_type) return true;
    if (from_type == SIMPLE(kExitCode) and
        (to_type == SIMPLE(kBool) or to_type == SIMPLE(kInt)))
        return true;
    if (from_type == SIMPLE(kRelPath) and to_type == SIMPLE(kPath)) return true;
    return false;
}

} // namespace

DeduceResult Deduce(const ast::Expression &expr, State &state) {

}

bool SatisfyRequirement(
    const ast::Expression &expr, Type::Pointer should_be, State &state) {
    auto result = Deduce(expr, state);
    if (result.fail) return false;
    if (result.empty_array and (should_be->ToMap() or should_be->ToArray())) {
        state.env.Insert(&expr, std::move(should_be));
        return true;
    }
    if (ImplicitConvertible(result.type.get(), should_be.get())) {
        state.env.Insert(&expr, std::move(result.type));
        return true;
    }
    state.TypeError(expr, Error::kInvalidType);
    return false;
}

} // namespace type

} // namespace sushi