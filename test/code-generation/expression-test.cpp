#include "gtest/gtest.h"
#include "util.h"
#include "../parser/util.h"
#include "sushi/code-generation/code-generation.h"

namespace sushi {
namespace code_generation {
namespace test {


} // namespace sushi
} // namespace code_generation
} // namespace test

using namespace sushi;
using namespace sushi::code_generation;
using namespace sushi::code_generation::test;

TEST(ExpressionTest, TestLiteral) {
    auto program = Parse("define a : Int = 123").program;
    auto scope = std::make_shared<scope::Scope>(nullptr);
    scope::Environment env;
    env.Insert(&program, scope);
    auto stmt = program.statements[0].get();

    // type::BuiltInAtom type(type::BuiltInAtom::Type::kInt);

    env.Insert(
        dynamic_cast<ast::VariableDef*>(stmt)->value.get(),
        std::make_unique<type::BuiltInAtom>(type::BuiltInAtom::Type::kInt));

    CodeGenerator generator;
    auto code = generator.GenCode(program, env);
    std::cout << code << std::endl;
}
