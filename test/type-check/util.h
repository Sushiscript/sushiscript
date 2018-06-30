#ifndef SUSHI_TEST_TYPE_CHECK_H_
#define SUSHI_TEST_TYPE_CHECK_H_

#include "sushi/ast/to-string.h"
#include "sushi/parser/parser.h"
#include "sushi/scope.h"
#include "sushi/type-system/type-checking.h"
#include "sushi/type-system/type.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std::string_literals;
using namespace sushi;

template <typename K, typename V>
using Table = std::unordered_map<K, V>;
using TypingTable = Table<const ast::Expression *, type::Type::Pointer>;
using StringTable = Table<std::string, std::string>;

namespace std {

inline std::ostream &operator<<(std::ostream &os, const StringTable &table) {
    os << "{\n";
    for (auto &kv : table) {
        os << "  " << kv.first << ": " << kv.second << '\n';
    }
    os << "}\n";
    return os;
}

inline std::ostream &
operator<<(std::ostream &os, const std::vector<parser::Error> &es) {
    for (auto &pe : es) {
        os << pe.ToString() << '\n';
    }
    return os;
}

inline std::ostream &
operator<<(std::ostream &os, const std::vector<type::Error> &es) {
    for (auto &te : es) {
        os << te.ToString() << '\n';
    }
    return os;
}

} // namespace std

namespace sushi {

inline StringTable InternalToString(const TypingTable &table) {
    StringTable tb;
    for (auto &kv : table) {
        std::string tp(kv.second == nullptr ? "null"s : kv.second->ToString());
        tb.emplace(ast::ToString(kv.first), std::move(tp));
    }
    return tb;
}

inline void
ExpectContainTypings(const TypingTable &typings, const StringTable &atleast) {
    auto actual = InternalToString(typings);
    bool success = true;
    for (auto &kv : atleast) {
        std::string deduced_type(
            actual.count(kv.first) ? actual[kv.first] : "N/A"s);
        std::string expected_type(kv.second);
        EXPECT_EQ(deduced_type, expected_type);
    }
    EXPECT_TRUE(success) << "complete typing table:\n" << actual;
}

// borrowed from test/parser/util.h
inline parser::ParsingResult Parse(const std::string &s) {
    std::istringstream iss(s);
    lexer::Lexer lexer(iss, {"", 1, 1});
    parser::Parser p(std::move(lexer));
    return p.Parse();
}

inline Environment GenerateScopeInfo(const ast::Program &program) {
    return {};
}

inline void WithTypeCheckResult(
    const std::string &source,
    std::function<void(const Environment &, const std::vector<type::Error> &)>
        f) {
    SCOPED_TRACE(source);
    auto pr = Parse(source);
    ASSERT_TRUE(pr.errors.empty()) << pr.errors;
    auto env = GenerateScopeInfo(pr.program);
    auto errors = type::Check(pr.program, env);
    f(env, errors);
}

inline void
TypingSuccess(const std::string &source, const StringTable &expect_typings) {
    WithTypeCheckResult(
        source, [&expect_typings](const auto &env, const auto &tes) {
            EXPECT_TRUE(tes.empty()) << tes;
            // TODO
            ExpectContainTypings({}, expect_typings);
        });
}

inline void TypingError(
    const std::string &source, type::Error::Tp t, const std::string &expr) {
    WithTypeCheckResult(source, [t, &expr](const auto &env, const auto &tes) {
        bool error_found = false;
        for (auto &te : tes) {
            if (te.type == t and ((te.expr == nullptr and expr.empty()) or
                                  (ast::ToString(te.expr) == expr))) {
                error_found = true;
                break;
            }
        }
        EXPECT_TRUE(error_found)
            << "error " << type::Error::ToString(t) << " not found in\n"
            << tes;
    });
}

} // namespace sushi

#endif // SUSHI_TEST_TYPE_CHECK_H_