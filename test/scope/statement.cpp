#include "util.h"

using namespace sushi::scope::test;

TEST(StatementTest, TestVariableDef) {
    VariableDefSuccess("define x = 1", { "x" });
    VariableDefSuccess("define x = 1; define y = 1;", { "x", "y" });
    VariableDefError("define x = 1; define x = 0", Error::Type::kRedefinedError);
}

TEST(StatementTest, TestVariableDef_New) {
    ScopeSuccess("define x = 1; define y = 2", {{"x", "y"}}, {});
}

// TEST(StatementTest, TestAssignment) {
//     std::string source_code = "x = 1";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("x");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
// }

// TEST(StatementTest, TestVariableDef) {
//     std::string source_code = "define x = 1";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("x");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
// }

// TEST(StatementTest, TestFunctionDef) {
//     std::string source_code =
//         "define f(x: Int, y: String, z: Array Int):Int = \n"
//         "    define x1 = 1 \n"
//         "    return z[x]";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("f");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
//     auto & func_stmt = program.statements[0];
//     auto & func_body = ((ast::FunctionDef *)func_stmt.get())->body;
//     auto func_scope = environment.LookUp(&func_body);
//     EXPECT_TRUE(func_scope != nullptr);
//     ident_info = func_scope->LookUp("x1");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == func_scope);
//     EXPECT_TRUE(func_scope->Outer() == top_scope);
// }

// TEST(StatementTest, TestIfStmt) {
//     std::string source_code =
//         "if x < 5:\n"
//         "  print x1\n"
//         "else:\n"
//         "  print x2";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("x");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
//     auto & if_stmt = program.statements[0];
//     auto & true_body = ((ast::IfStmt *)if_stmt.get())->true_body;
//     auto & false_body = ((ast::IfStmt *)if_stmt.get())->false_body;
//     auto true_scope = environment.LookUp(&true_body);
//     auto false_scope = environment.LookUp(&false_body);
//     EXPECT_TRUE(true_scope != nullptr);
//     EXPECT_TRUE(false_scope != nullptr);

//     ident_info = true_scope->LookUp("x1");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == true_scope);
//     EXPECT_TRUE(true_scope->Outer() == top_scope);

//     ident_info = false_scope->LookUp("x2");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == false_scope);
//     EXPECT_TRUE(false_scope->Outer() == top_scope);
// }

// TEST(StatementTest, TestReturnStmt) {
//     std::string source_code = "return x";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("x");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
// }

// TEST(StatementTest, TestSwitchStmt) {
//     std::string source_code =
//         "switch x\n"
//         "  case 0\n"
//         "    print x\n"
//         "  case 1\n"
//         "    print x\n"
//         "  default\n"
//         "    print y";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("x");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
// }

// TEST(StatementTest, TestForStmt) {
//     std::string source_code = "for i < 4: continue";
//     auto program = Parse(source_code).program;
//     auto environment = ScopeCheck(program);
//     auto top_scope = environment.LookUp(&program);
//     auto ident_info = top_scope->LookUp("i");
//     EXPECT_TRUE(ident_info != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope != nullptr);
//     EXPECT_TRUE(ident_info->defined_scope == top_scope);
// }

// TEST(StatementTest, LoopControlStmt) {
//     // pass
// }

// TEST(StatementTest, TestExpression) {
//     // pass
// }
