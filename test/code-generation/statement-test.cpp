#include "./util.h"
#include "gtest/gtest.h"

using namespace sushi::code_generation::test;

TEST(StatementTest, VariableDef) {
    CodeGenSuccess("define a : Int = 1", R"(_sushi_t_=$((1))
local a=${_sushi_t_}

unset a
)");
    CodeGenSuccess("define a = 1", R"(_sushi_t_=$((1))
local a=${_sushi_t_}

unset a
)");
}
