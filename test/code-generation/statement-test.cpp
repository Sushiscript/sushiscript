#include "./util.h"
#include "gtest/gtest.h"

using namespace sushi::code_generation::test;

TEST(StatementTest, VariableDef) {
//     CodeGenSuccess("define a : Int = 1", R"(_sushi_t_=$((1))
// local a=${_sushi_t_}

// unset a)");
//     CodeGenSuccess("define a = 1", R"(_sushi_t_=$((1))
// local a=${_sushi_t_}

// unset a)");
}

// TEST(StatementTest, Interpolation) {
//     CodeGenSuccess("define a = 1\n\"${a}\"", R"(local _sushi_t_=$((1))
// local a=${_sushi_t_}
// local _sushi_t_0_="${a}"
// local _sushi_t_1_=${_sushi_t_0_})");
// }

// TEST(StatementTest, Command) {
//     CodeGenSuccess("! echo \"hello\"", R"(local _sushi_t_="echo"
// local _sushi_t_0_="hello"
// ${_sushi_t_} ${_sushi_t_0_}
// local _sushi_t_1_=$?

// unset _sushi_t_0_
// unset _sushi_t_
// unset _sushi_t_1_)");
// }
