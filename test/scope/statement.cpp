#include "util.h"

TEST(StatementTest, TestVariableDef) {
    std::string source_code = "define x = 1";
    auto program = Parse(source_code).program;
    auto environment = TypeCheck(program);
    auto top_scope = environment.LookUp(&program);
    auto ident_info = top_scope->LookUp("x");
    EXPECT_TRUE(ident_info != nullptr);
    EXPECT_TRUE(ident_info->defined_scope != nullptr);
    EXPECT_TRUE(ident_info->defined_scope == top_scope);
}

TEST(StatementTest, TestFunctionDef) {
    ParseSuccess("define f() = return 1", "define f() =\n  return 1");
    ParseSuccess("define f() =\n return 1", "define f() =\n  return 1");
    ParseSuccess(
        "export define f() = return 1", "export define f() =\n  return 1");
    ParseSuccess("define f( ) = return 1", "define f() =\n  return 1");
    ParseSuccess("define f(): Int = return 1", "define f(): Int =\n  return 1");
    ParseSuccess(
        "define f(x: Int): Int = return x",
        "define f(x: Int): Int =\n  return x");
    ParseSuccess(
        "define f(x: Int, y: String): Int = return length y",
        "define f(x: Int, y: String): Int =\n  return (length y)");
    ParseSuccess(
        "define f(x: Int, y: String, z: Array Int): Int =\n return z[x]",
        "define f(x: Int, y: String, z: (Array Int)): Int =\n  return z[x]");
}
