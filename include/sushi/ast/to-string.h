#ifndef SUSHI_AST_TO_STRING_H_
#define SUSHI_AST_TO_STRING_H_

#include "boost/algorithm/string/join.hpp"
#include "boost/format.hpp"
#include "sushi/ast/expression.h"
#include "sushi/ast/statement.h"
#include <string>

namespace sushi {
namespace ast {

inline std::string ToString(FdLit::Value fd) {
    using V = FdLit::Value;
    switch (fd) {
    case V::kStdin: return "stdin";
    case V::kStdout: return "stdout";
    case V::kStderr: return "stderr";
    }
}

inline std::string ToString(UnaryExpr::Operator op) {
    using O = UnaryExpr::Operator;
    switch (op) {
    case O::kNot: return "not";
    case O::kNeg: return "-";
    case O::kPos: return "+";
    }
}

inline std::string ToString(BinaryExpr::Operator op) {
    using O = BinaryExpr::Operator;
    switch (op) {
    case O::kAdd: return "+";
    case O::kMinus: return "-";
    case O::kMult: return "*";
    case O::kDiv: return "//";
    case O::kMod: return "%";
    case O::kLess: return "<";
    case O::kGreat: return ">";
    case O::kLessEq: return "<=";
    case O::kGreatEq: return ">=";
    case O::kEqual: return "==";
    case O::kNotEq: return "!=";
    case O::kAnd: return "and";
    case O::kOr: return "or";
    }
}

inline std::string ToString(type::BuiltInAtom::Type t) {
    return type::BuiltInAtom::ToString(t);
}

inline std::string ToString(Redirection::Direction dir) {
    return dir == Redirection::Direction::kIn ? "from" : "to";
}

std::string ToString(const Literal *lit);

std::string ToString(const Expression *expr);

std::string ToString(const InterpolatedString &str);

std::string ToString(const Statement *stmt, int indent);

std::string ToString(const CommandLike *cmd);

std::string ToString(const TypeExpr *type);

inline std::string ToString(const Redirection &redir) {
    std::string dest =
        redir.external ? ToString(redir.external.get()) : std::string("here");
    std::string append = redir.append ? " append" : "";
    return (boost::format("%s %s %s%s") % ToString(redir.me) %
            ToString(redir.direction) % dest % append)
        .str();
}

inline std::string ToString(const Program &p, int indent) {
    std::vector<std::string> stmts;
    for (auto &stmt : p.statements) {
        stmts.push_back(ToString(stmt.get(), indent));
    }
    return boost::join(stmts, "\n");
}

namespace to_string {

struct Result {
    std::string result;
};

struct Literal2Str : LiteralVisitor::Const, Result {
    SUSHI_VISITING(IntLit, i) {
        result = std::to_string(i.value);
    }
    SUSHI_VISITING(BoolLit, b) {
        if (b.value)
            result = "true";
        else
            result = "false";
    }
    SUSHI_VISITING(CharLit, c) {
        result = std::string("\'") + c.value + '\'';
    }
    SUSHI_VISITING(UnitLit, u) {
        result = "()";
    }
    SUSHI_VISITING(FdLit, fd) {
        result = ToString(fd.value);
    }
    SUSHI_VISITING(StringLit, str) {
        result = (boost::format("\"%s\"") % ToString(str.value)).str();
    }
    SUSHI_VISITING(PathLit, p) {
        result = ToString(p.value);
    }
    SUSHI_VISITING(RelPathLit, p) {
        result = ToString(p.value);
    }
    SUSHI_VISITING(ArrayLit, a) {
        std::vector<std::string> elems;
        for (auto &e : a.value) elems.push_back(ToString(e.get()));
        result = (boost::format("{%s}") % (boost::join(elems, ", "))).str();
    }
    SUSHI_VISITING(MapLit, m) {
        std::vector<std::string> maps;
        for (auto &p : m.value)
            maps.push_back((boost::format("%s: %s") % ToString(p.first.get()) %
                            ToString(p.second.get()))
                               .str());
        result = (boost::format("{%s}") % (boost::join(maps, ", "))).str();
    }
};

struct Command2Str : CommandLikeVisitor::Const, Result {
    SUSHI_VISITING(FunctionCall, f) {
        std::vector<std::string> params;
        params.push_back(f.func.name);
        for (auto &p : f.parameters) {
            params.push_back(ToString(p.get()));
        }
        result = boost::join(params, " ");
    }
    SUSHI_VISITING(Command, c) {
        std::vector<std::string> params;
        params.push_back(ToString(c.cmd));
        for (auto &p : c.parameters) {
            params.push_back(ToString(p));
        }
        result = (boost::format("! %s") % boost::join(params, " ")).str();
    }
};

struct Expression2Str : ExpressionVisitor::Const, Result {
    SUSHI_VISITING(Literal, l) {
        result = ToString(&l);
    }
    SUSHI_VISITING(UnaryExpr, u) {
        result =
            (boost::format("(%s %s)") % ToString(u.op) % ToString(u.expr.get()))
                .str();
    }
    SUSHI_VISITING(BinaryExpr, b) {
        result = (boost::format("(%s %s %s)") % ToString(b.lhs.get()) %
                  ToString(b.op) % ToString(b.rhs.get()))
                     .str();
    }
    SUSHI_VISITING(Variable, v) {
        result = v.var.name;
    }
    SUSHI_VISITING(Indexing, i) {
        result = (boost::format("%s[%s]") % ToString(i.indexable.get()) %
                  ToString(i.index.get()))
                     .str();
    }
    SUSHI_VISITING(CommandLike, cmd) {
        std::vector<std::string> pipes;
        for (auto cur = &cmd; cur != nullptr; cur = cur->pipe_next.get()) {
            std::vector<std::string> redirs;
            auto call = ToString(cur);
            for (auto &redir : cur->redirs) redirs.push_back(ToString(redir));
            if (redirs.empty())
                pipes.push_back(std::move(call));
            else
                pipes.push_back((boost::format("%s redirect %s") %
                                 std::move(call) % boost::join(redirs, ", "))
                                    .str());
        }
        result = (boost::format("(%s)") % boost::join(pipes, " | ")).str();
    }
};

struct Statement2Str : StatementVisitor::Const, Result {
    Statement2Str(int indent) : indent(indent) {}

    SUSHI_VISITING(Assignment, ass) {
        result =
            Indent() + (boost::format("%s = %s") % ToString(ass.lvalue.get()) %
                        ToString(ass.value.get()))
                           .str();
    }

    SUSHI_VISITING(VariableDef, vd) {
        std::string export_ = vd.is_export ? "export " : "";
        std::string type = vd.type ? ": " + ToString(vd.type.get()) : "";
        result = Indent() + (boost::format("%sdefine %s%s = %s") % export_ %
                             vd.name % type % ToString(vd.value.get()))
                                .str();
    }
    SUSHI_VISITING(FunctionDef, fd) {
        std::vector<std::string> params;
        std::string export_ = fd.is_export ? "export " : "";
        std::string ret_type =
            fd.ret_type ? ": " + ToString(fd.ret_type.get()) : "";
        std::string body = ToString(fd.body, 2 + indent);
        for (auto &param : fd.params) {
            params.push_back((boost::format("%s: %s") % param.name %
                              ToString(param.type.get()))
                                 .str());
        }
        result =
            Indent() + (boost::format("%sdefine %s(%s)%s =\n%s") % export_ %
                        fd.name % boost::join(params, ", ") % ret_type % body)
                           .str();
    }
    SUSHI_VISITING(IfStmt, fi) {
        auto else_ = not fi.false_body.statements.empty()
                         ? (boost::format("\n%selse\n%s") % Indent() %
                            ToString(fi.false_body, indent + 2))
                               .str()
                         : "";
        result = Indent() +
                 (boost::format("if %s\n%s%s") % ToString(fi.condition.get()) %
                  ToString(fi.true_body, indent + 2) % else_)
                     .str();
    }
    SUSHI_VISITING(ForStmt, fo) {
        std::string cond =
            (fo.condition.IsRange() ? fo.condition.ident_name + " in " : "") +
            ToString(fo.condition.condition.get());
        result = Indent() + (boost::format("for %s\n%s") % cond %
                             ToString(fo.body, indent + 2))
                                .str();
    }
    SUSHI_VISITING(SwitchStmt, sw) {
        std::vector<std::string> cases;
        for (auto &case_ : sw.cases) {
            auto fmt =
                boost::format("%s\n%s") %
                (case_.condition ? "case " + ToString(case_.condition.get())
                                 : std::string("default")) %
                ToString(case_.body, indent + 2);
            cases.push_back(Indent() + ' ' + fmt.str());
        }
        result =
            Indent() + (boost::format("switch %s\n%s") %
                        ToString(sw.switched.get()) % boost::join(cases, "\n"))
                           .str();
    }
    SUSHI_VISITING(ReturnStmt, ret) {
        std::string value = ret.value == nullptr
                                ? std::string()
                                : ' ' + ToString(ret.value.get());
        result = Indent() + "return" + value;
    }

    SUSHI_VISITING(LoopControlStmt, l) {
        std::string t = l.control_type == LoopControlStmt::Value::kBreak
                            ? "break"
                            : "continue";
        std::string level = l.level == 1 ? "" : " " + std::to_string(l.level);
        result = Indent() + t + level;
    }
    SUSHI_VISITING(Expression, e) {
        result = Indent() + ToString(&e);
    }

    std::string Indent() {
        return std::string(indent, ' ');
    }
    int indent;
};

struct Type2Str : TypeExprVisitor::Const, Result {
    SUSHI_VISITING(TypeLit, t) {
        result = ToString(t.type);
    }
    SUSHI_VISITING(ArrayType, a) {
        result = "(Array " + ToString(a.element) + ")";
    }
    SUSHI_VISITING(MapType, m) {
        result =
            (boost::format("(Map %s %s)") % ToString(m.key) % ToString(m.value))
                .str();
    }
    SUSHI_VISITING(FunctionType, f) {
        std::vector<std::string> params;
        params.push_back(ToString(f.result.get()));
        for (auto &p : f.params) {
            params.push_back(ToString(p.get()));
        }
        result = "(Function " + boost::join(params, " ") + ')';
    }
};

} // namespace to_string

inline std::string ToString(const Literal *lit) {
    to_string::Literal2Str v;
    lit->AcceptVisitor(v);
    return v.result;
}

inline std::string ToString(const InterpolatedString &str) {
    std::string result;
    str.Traverse(
        [&](const std::string &s) { result += s; },
        [&](const Expression &expr) {
            result += "${" + ToString(&expr) + '}';
        });
    return result;
}

inline std::string ToString(const CommandLike *cmd) {
    to_string::Command2Str v;
    cmd->AcceptVisitor(v);
    return v.result;
}

inline std::string ToString(const Statement *stmt, int indent) {
    to_string::Statement2Str v(indent);
    stmt->AcceptVisitor(v);
    return v.result;
}

inline std::string ToString(const Expression *expr) {
    to_string::Expression2Str v;
    expr->AcceptVisitor(v);
    return v.result;
}

inline std::string ToString(const TypeExpr *type) {
    to_string::Type2Str v;
    type->AcceptVisitor(v);
    return v.result;
}

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_TO_STRING_H_