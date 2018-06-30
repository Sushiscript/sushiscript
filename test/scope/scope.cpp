#include "util.h"

TEST(SCOPE_TOP_TEST, TEST_TOP_PRPGRAM_SCOPE) {
    auto program = Parse("for i < 4: i = i + 1").program;
    auto environment = ScopeCheck(program);
    auto top_scope = environment.LookUp(&program);
    EXPECT_TRUE(top_scope != nullptr);
}

// int main(int argc, char **argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
