#include "../parser/util.h"
#include "sushi/lexer.h"
#include "sushi/parser/parser.h"
#include "sushi/scope.h"
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

inline void VariableDefError(const std::string &source, const Error::Type &err_type) {
    SimpleWithScopeCheckResult(
        source, [&err_type](const Scope *scope, const Environment &env,
                    const std::vector<Error> &errs) {
            bool found_err = false;
            for (auto & err : errs) {
                if (err.type == err_type) {
                    found_err = true;
                    break;
                }
            }
            EXPECT_TRUE(found_err);
        });
}

} // namespace test
} // namespace scope
} // namespace sushi
