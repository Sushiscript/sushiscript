#include "gtest/gtest.h"
#include "sushi/lexer.h"
#include "sushi/scope/environment.h"
#include "sushi/scope/statement-visitor.h"
#include "sushi/parser/parser.h"

using namespace sushi::scope;

TEST(SCOPE_TEST_1, ASSRET_TRUE) {
    // std::string s("for i < 4: i = i + 1");
    // std::istringstream iss(s);
    // sushi::lexer::Lexer lexer(iss, {"", 1, 1});
    // sushi::parser::Parser p(std::move(lexer));

    // Environment environment(p.Parse().program);
    Environment environment;
    StatementVisitor visitor(environment);
    EXPECT_FALSE(false);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
