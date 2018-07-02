#include "./util.h"
#include "gtest/gtest.h"

using namespace sushi::code_generation::test;

TEST(StatementTest, VariableDef) {
    CodeGenSuccess("define a : Int = 1", R"(main() {
    _sushi_t_=$((1))
    local a=${_sushi_t_}

    unset a

}
main
)");
    CodeGenSuccess("define a = 1", R"(main() {
    _sushi_t_=$((1))
    local a=${_sushi_t_}

    unset a

}
main
)");
}
