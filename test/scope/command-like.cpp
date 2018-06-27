#include "util.h"

TEST(CommandLikeVisitorTest, FunctionCallTest) {
    std::string source_code = "x";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    // auto ident_info = top_scope->LookUp("x");
    // EXPECT_TRUE(ident_info != nullptr);
    // EXPECT_TRUE(ident_info->defined_scope != nullptr);
    // EXPECT_TRUE(ident_info->defined_scope == top_scope);
}

TEST(CommandLikeVisitorTest, TestCommand) {
    std::string source_code = "! hello${world} ${world and you}; ";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    // auto ident_info = top_scope->LookUp("world");
    // EXPECT_TRUE(ident_info != nullptr);
    // EXPECT_TRUE(ident_info->defined_scope != nullptr);
    // EXPECT_TRUE(ident_info->defined_scope == top_scope);
}
