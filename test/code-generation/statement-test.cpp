#include "./util.h"
#include "gtest/gtest.h"

using namespace sushi::code_generation::test;

TEST(StatementTest, VariableDef) {
    CodeGenSuccess("define a = 1", "local a=$((1))");
}
