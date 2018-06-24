#include "sushi/scope.h"
#include "../parser/util.h"
#include "sushi/lexer.h"
#include "sushi/parser/parser.h"
#include "gtest/gtest.h"

using namespace sushi::scope;

TEST(SCOPE_TEST_1, ASSRET_TRUE) {
    auto program = Parse("for i < 4: i = i + 1").program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    EXPECT_TRUE(top_scope != nullptr);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
