#ifndef SUSHI_TEST_CODE_GEN_UTIL_H_
#define SUSHI_TEST_CODE_GEN_UTIL_H_

#include "sushi/scope.h"
#include "sushi/parser/parser.h"
#include "gtest/gtest.h"
#include "../parser/util.h"
#include "sushi/code-generation/code-generation.h"
#include "sushi/type-system/type-checking.h"
#include "sushi/type-system/type.h"

namespace sushi {
namespace code_generation {
namespace test {

inline void CodeGenSuccess(const std::string &source, const std::string &expect_res) {
    auto res = Parse(source);
    ASSERT_TRUE(res.errors.empty());

    scope::Environment env;
    auto scope_errs = scope::ScopeCheck(res.program, env);
    ASSERT_TRUE(scope_errs.empty());

    auto type_errs = type::Check(res.program, env);
    ASSERT_TRUE(type_errs.empty());

    CodeGenerator code_gen;
    auto code = code_gen.GenCode(res.program, env);
    EXPECT_EQ(code, CodeGenerator::DecorateTopCode(expect_res));
}

} // namespace test
} // namespace code_generation
} // namespace sushi

#endif
