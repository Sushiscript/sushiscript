#ifndef SUSHI_SCOPE_TEST_UTIL_H
#define SUSHI_SCOPE_TEST_UTIL_H

#include "../parser/util.h"
#include "sushi/lexer.h"
#include "sushi/parser/parser.h"
#include "sushi/scope.h"
#include "visitor/include/statement-visitor.h"
#include "gtest/gtest.h"

using namespace sushi::scope;

namespace std {

inline std::ostream &
operator<<(std::ostream &os, const std::vector<Error> &es) {
    for (auto &te : es) {
        os << te.ToString() << '\n';
    }
    return os;
}

} // namespace std

namespace sushi {
namespace scope {
namespace test {

// borrowed from test/parser/util.h
inline parser::ParsingResult Parse(const std::string &s) {
    std::istringstream iss(s);
    lexer::Lexer lexer(iss, {"", 1, 1});
    parser::Parser p(std::move(lexer));
    return p.Parse();
}

inline void SimpleWithScopeCheckResult(
    const std::string &source,
    std::function<
        void(const Scope *, const Environment &, const std::vector<Error> &)>
        f) {
    SCOPED_TRACE(source);
    auto pr = Parse(source);
    ASSERT_TRUE(pr.errors.empty()) << pr.errors;
    Environment env;
    auto errs = ScopeCheck(pr.program, env);
    auto scope = env.LookUp(&pr.program);

    ASSERT_TRUE(scope != nullptr);

    f(scope, env, errs);
}

inline void VariableDefSuccess(
    const std::string &source,
    const std::vector<std::string> &expected_scope_ids) {
    SimpleWithScopeCheckResult(
        source, [&expected_scope_ids](
                    const Scope *scope, const Environment &env,
                    const std::vector<Error> &errs) {
            ASSERT_TRUE(errs.empty()) << errs;
            for (auto &id : expected_scope_ids) {
                auto res = scope->LookUp(id);
                ASSERT_TRUE(res != nullptr);
                EXPECT_TRUE(res->defined_scope == scope);
            }
        });
}

inline void
VariableDefError(const std::string &source, const Error::Type &err_type) {
    SimpleWithScopeCheckResult(
        source, [&err_type](
                    const Scope *scope, const Environment &env,
                    const std::vector<Error> &errs) {
            bool found_err = false;
            for (auto &err : errs) {
                if (err.type == err_type) {
                    found_err = true;
                    break;
                }
            }
            EXPECT_TRUE(found_err);
        });
}

struct PretestResult {
    std::vector<const ast::Program *> programs;
    std::vector<const ast::Identifier *> identifiers;
};

inline PretestResult Pretest(const ast::Program &program) {
    PretestResult res;
    res.programs.push_back(&program);

    StatementVisitor visitor;

    for (auto &stmt : program.statements) {
        stmt->AcceptVisitor(visitor);
    }

    MergeVector(res.identifiers, visitor.identifiers);
    MergeVector(res.programs, visitor.programs);

    return res;
}

inline void ScopeTest(
    const std::vector<const Scope *> &scopes,
    const std::vector<std::vector<std::string>> &expect_scope_def_ids) {
    for (int i = 0; i < scopes.size(); ++i) {
        auto &scope = scopes[i];
        auto &def_ids = expect_scope_def_ids[i];
        for (auto &id : def_ids) {
            auto ident_info = scope->LookUp(id);
            ASSERT_TRUE(ident_info != nullptr) << "In scope " << i
                << " Finding " << id;
            EXPECT_TRUE(ident_info->defined_scope == scope) << "In scope " << i
                << " Finding " << id;
        }
    }
}

inline int FindScope(const Environment &env, const std::vector<const Scope *> &scopes, const ast::Identifier *id) {
    auto scope = env.LookUp(id);
    for (int i = 0; i < scopes.size(); ++i) {
        if (scope == scopes[i]) return i;
    }
    return -1;
}

inline void EnvironTest(
    const Environment &env, const std::vector<const Scope *> &scopes,
    const std::vector<const ast::Identifier *> &ids,
    const std::vector<int> &use_scope_index) {
    for (int i = 0; i < ids.size(); ++i) {
        auto scope = env.LookUp(ids[i]);
        ASSERT_TRUE(scope != nullptr);
        ASSERT_TRUE(i < use_scope_index.size());
        ASSERT_TRUE(use_scope_index[i] < scopes.size());
        EXPECT_TRUE(scope == scopes[use_scope_index[i]])
            << "Expect " << i << ": " << ids[i]->name << " in scope " << use_scope_index[i] << '\n'
            << "But in " << FindScope(env, scopes, ids[i]);
    }
}

inline void ScopeSuccess(
    const std::string &source,
    const std::vector<std::vector<std::string>> &expect_scope_def_ids,
    const std::vector<int> &use_scope_index) {
    SCOPED_TRACE(source);
    auto pr = Parse(source);
    ASSERT_TRUE(pr.errors.empty()) << pr.errors;
    Environment env;
    auto errs = ScopeCheck(pr.program, env);
    ASSERT_TRUE(errs.empty()) << errs;

    auto res = Pretest(pr.program);

    std::vector<const Scope *> scopes;
    // Get scopes
    for (auto &program : res.programs) {
        scopes.push_back(env.LookUp(program));
    }

    ScopeTest(scopes, expect_scope_def_ids);
    EnvironTest(env, scopes, res.identifiers, use_scope_index);
}

inline void ScopeError(const std::string &source, const Error::Type &err_type) {
    SCOPED_TRACE(source);
    auto pr = Parse(source);
    ASSERT_TRUE(pr.errors.empty()) << pr.errors;
    Environment env;
    auto errs = ScopeCheck(pr.program, env);

    ASSERT_FALSE(errs.empty()) << "no error, it's weired";
    bool found_err = false;
    for (auto &err : errs) {
        if (err.type == err_type) {
            found_err = true;
            break;
        }
    }
    EXPECT_TRUE(found_err);
}

} // namespace test
} // namespace scope
} // namespace sushi

#endif
