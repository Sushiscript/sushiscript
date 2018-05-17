#include "./util.h"
#include "gtest/gtest.h"

TEST(ExpressionTest, TestLiteral) {
    ParseSuccess("123", "123");
    ParseSuccess("123   ", "123");
    ParseSuccess("0  \n\n\n ", "0");
    ParseSuccess("stdout", "stdout");
    ParseSuccess("()", "()");
    ParseSuccess("'a'", "'a'");
    ParseSuccess("\"abcd\"", "\"abcd\"");
    ParseSuccess("../hello/world", "../hello/world");
}