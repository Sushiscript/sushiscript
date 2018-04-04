#ifndef SUSHI_AST_EXPRESSION_LITERAL_H_
#define SUSHI_AST_EXPRESSION_LITERAL_H_

#include "./expression.h"
#include "sushi/ast/interpolated-string.h"
#include "sushi/util/visitor.h"
#include <vector>

namespace sushi {

struct IntLit;
struct BoolLit;
struct UnitLit;
struct FdLit;
struct StringLit;
struct PathLit;
struct ArrayLit;
struct MapLit;

using LiteralVisitor = sushi::util::DefineVisitor<
    IntLit, BoolLit, UnitLit, FdLit, StringLit, PathLit, ArrayLit, MapLit>;

struct Literal : public Expression {
    SUSHI_ACCEPT_VISITOR_FROM(Expression)

    SUSHI_VISITABLE(LiteralVisitor);
};

struct IntLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);

    IntLit(int value) : value(value) {}

    int value;
};

struct BoolLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);

    BoolLit(bool value) : value(value) {}

    bool value;
};

struct UnitLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);
};

struct FdLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);
    enum struct Value { kStdin, kStdout, kStderr };

    FdLit(FdLit::Value value) : value(value) {}

    FdLit::Value value;
};

struct StringLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);

    StringLit(InterpolatedString value) : value(std::move(value)) {}

    InterpolatedString value;
};

struct PathLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);

    PathLit(InterpolatedString value) : value(std::move(value)) {}

    InterpolatedString value;
};

struct ArrayLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);

    ArrayLit(std::vector<std::unique_ptr<Expression>> value)
        : value(std::move(value)) {}

    std::vector<std::unique_ptr<Expression>> value;
};

struct MapLit : public Literal {
    SUSHI_ACCEPT_VISITOR_FROM(Literal);
    using MapItem =
        std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>;

    MapLit(std::vector<MapItem> value) : value(std::move(value)) {}

    std::vector<MapItem> value;
};

} // namespace sushi

#endif // SUSHI_AST_EXPRESSION_LITERAL_H_
