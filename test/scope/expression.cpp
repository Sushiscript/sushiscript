#include "util.h"

TEST(ExpressionTest, TestVariable) {
    std::string source_code = "x";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    auto ident_info = top_scope->LookUp("x");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);
}

TEST(ExpressionTest, TestLiteral) {
    std::string source_code = "123";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
}

TEST(ExpressionTest, TestUnaryExpr) {
    std::string source_code = "+ x";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    auto ident_info = top_scope->LookUp("x");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);
}

TEST(ExpressionTest, TestBinaryExpr) {
    std::string source_code = "x + y";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    auto ident_info = top_scope->LookUp("x");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);

    ident_info = top_scope->LookUp("y");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);
}

TEST(ExpressionTest, TestCommandLike) {
    // pass
}

TEST(ExpressionTest, Indexing) {
    std::string source_code = "{1,2,3}[x]";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    auto ident_info = top_scope->LookUp("x");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);
}
