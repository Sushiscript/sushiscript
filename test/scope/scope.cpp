#include "util.h"

TEST(SCOPE_TOP_TEST, TEST_TOP_PRPGRAM_SCOPE) {
    auto program = Parse("for i < 4: i = i + 1").program;
    Environment env;
    auto errs = ScopeCheck(program, env);
    auto top_scope = env.LookUp(&program);
    EXPECT_TRUE(top_scope != nullptr);
}

// int main(int argc, char **argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
