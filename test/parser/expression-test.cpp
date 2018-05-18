#include "./util.h"
#include "gtest/gtest.h"

TEST(ExpressionTest, TestLiteral) {
    ParseSuccess("ident", "ident");
    ParseSuccess("123", "123");
    ParseSuccess("123   ", "123");
    ParseSuccess("0  \n\n\n ", "0");
    ParseSuccess("stdout", "stdout");
    ParseSuccess("()", "()");
    ParseSuccess("'a'", "'a'");
    ParseSuccess("\"\"", "\"\"");
    ParseSuccess("\"abcd\"", "\"abcd\"");
    ParseSuccess("../hello/world", "../hello/world");
}

TEST(ExpressionTest, TestUnaryOperation) {
    ParseSuccess("- 1", "(- 1)");
    ParseSuccess("--1", "(- (- 1))");
    ParseSuccess("----1", "(- (- (- (- 1))))");
    ParseSuccess("not true", "(not true)");
    ParseSuccess("+ 100", "(+ 100)");
}

TEST(ExpressionTest, SimpleBinaryOperation) {
    ParseSuccess("1 + 2", "(1 + 2)");
    ParseSuccess("1 - 2 - 3", "((1 - 2) - 3)");
    ParseSuccess("1 - 2 - 3 - 4", "(((1 - 2) - 3) - 4)");
    ParseSuccess("true or false", "(true or false)");
    ParseSuccess("./hello // ./world", "(./hello // ./world)");
    ParseSuccess("1 + 2 * 3", "(1 + (2 * 3))");
    ParseSuccess("1 * 2 + 3", "((1 * 2) + 3)");
    ParseSuccess("1 * 2 + 3 * 4", "((1 * 2) + (3 * 4))");
    ParseSuccess("1*2+3*4", "((1 * 2) + (3 * 4))");
    ParseSuccess(
        "1 + 2 * 3 > 4 or 5 // 6 < 7 + 8",
        "(((1 + (2 * 3)) > 4) or ((5 // 6) < (7 + 8)))");
    ParseSuccess(
        "1 + 2 * 3 > 4 == 5 // 6 < 7 + 8 and 1 * 2 + 3",
        "((((1 + (2 * 3)) > 4) == ((5 // 6) < (7 + 8))) and ((1 * 2) + 3))");
}

TEST(ExpressionTest, ParenAndBinary) {
    ParseSuccess("(1 + 2)", "(1 + 2)");
    ParseSuccess("(1 + 2) * 3", "((1 + 2) * 3)");
    ParseSuccess(
        "(1 + 2) * 3 > (4 or 5) // (6 < 7 + 8)",
        "(((1 + 2) * 3) > ((4 or 5) // (6 < (7 + 8))))");
}

TEST(ExpressionTest, UnaryAndBinary) {
    ParseSuccess("(1 + + 2)", "(1 + (+ 2))");
    ParseSuccess("(-1 + + 2) * -3", "(((- 1) + (+ 2)) * (- 3))");
    ParseSuccess(
        "not true or not false and not true",
        "(((not true) or (not false)) and (not true))");
}

TEST(ExpressionTest, TestInterpolation) {
    ParseSuccess(R"("${world}")", R"("${world}")");
    ParseSuccess(R"("hello${world}")", R"("hello${world}")");
    ParseSuccess(R"("${hello}world")", R"("${hello}world")");
    ParseSuccess(R"("hello${hello}world")", R"("hello${hello}world")");
    ParseSuccess(R"("hello${1 + 1}world")", R"("hello${(1 + 1)}world")");
    ParseSuccess(R"("hello${"hello"}world")", R"("hello${"hello"}world")");
    ParseSuccess(R"(../${hello}/world)", R"(../${hello}/world)");
    ParseSuccess(R"(../${"${hello}"}/world)", R"(../${"${hello}"}/world)");
}

TEST(ExpressionTest, TestFunctionCall) {
    ParseSuccess("function call", "(function call)");
    ParseSuccess("function()", "(function ())");
    ParseSuccess("func1() + func2()", "((func1 ()) + (func2 ()))");
    ParseSuccess("../${func1()}", "../${(func1 ())}");
}

TEST(ExpressionTest, TestArrayLiteral) {
    ParseSuccess("{}", "{}");
    ParseSuccess("{1}", "{1}");
    ParseSuccess("{1,2,3,4}", "{1, 2, 3, 4}");
    ParseSuccess(
        "{1 + 2,2 + 3,(3 - 4),4 + 5 * 6}",
        "{(1 + 2), (2 + 3), (3 - 4), (4 + (5 * 6))}");
}

TEST(ExpressionTest, TestMapLiteral) {
    ParseSuccess("{1:2}", "{1: 2}");
    ParseSuccess("{1:2, 3:4}", "{1: 2, 3: 4}");
    ParseSuccess(
        "{1 + 1:2 + 2, 3 + 3:4 + 4}", "{(1 + 1): (2 + 2), (3 + 3): (4 + 4)}");
    ParseSuccess(
        "{func():2 + 2 or 4 + 4, () : ()}",
        "{(func ()): ((2 + 2) or (4 + 4)), (): ()}");
}

TEST(ExpressionTest, TestIndexing) {
    ParseSuccess("123[1]", "123[1]");
    ParseSuccess(R"("hello"[1])", R"("hello"[1])");
    ParseSuccess(R"("hello"[1][1][1])", R"("hello"[1][1][1])");
    ParseSuccess(
        R"("hello"["hello"[1]][()])", R"("hello"["hello"[1]][()])");
    ParseSuccess("{1,2,3}[expr]", "{1, 2, 3}[expr]");
    ParseSuccess("{1,2,3}[func call]", "{1, 2, 3}[(func call)]");
    ParseSuccess("array[func call]", "array[(func call)]");
    ParseSuccess("(func call)[func call]", "(func call)[(func call)]");
}

TEST(ExpressionTest, TestCommand) {
    ParseSuccess("! hello", "(! hello;)");
    ParseSuccess("! hello;\n", "(! hello;)");
    ParseSuccess("! hello hello", "(! hello hello;)");
    ParseSuccess("! hello hello; ", "(! hello hello;)");
    ParseSuccess("! hello${world}; ", "(! hello${world};)");
    ParseSuccess(
        "! hello${world} ${world and you}; ",
        "(! hello${world} ${(world and you)};)");
    ParseSuccess("! hello --verbose --help", "(! hello --verbose --help;)");
    ParseSuccess(
        "not (! hello;) or not (! world;)",
        "((not (! hello;)) or (not (! world;)))");
}

TEST(ExpressionTest, TestPipe) {
    ParseSuccess("func call | another call", "(func call | another call)");
    ParseSuccess(
        "func call | another call | third call",
        "(func call | another call | third call)");
    ParseSuccess("! ls -1; | ! sort -n", "(! ls -1; | ! sort -n;)");
    ParseSuccess(
        "! ls -1; | ! sort -n; | ! pbcopy",
        "(! ls -1; | ! sort -n; | ! pbcopy;)");
    ParseSuccess(
        "! ls -1 | ! sort -n | ! pbcopy",
        "(! ls -1; | ! sort -n; | ! pbcopy;)");
    ParseSuccess("func call | ! a command", "(func call | ! a command;)");
    ParseSuccess("! a command; | func call", "(! a command; | func call)");
    ParseSuccess(
        "func call | ! a command; | func call",
        "(func call | ! a command; | func call)");
    ParseSuccess(
        "! a command; | func call | ! another command",
        "(! a command; | func call | ! another command;)");
}

TEST(ExpressionTest, TestRedirection) {
    ParseSuccess(
        "func call redirect to ./output",
        "(func call redirect stdout to ./output)");
    ParseSuccess(
        "! some command; redirect to ./output",
        "(! some command; redirect stdout to ./output)");
    ParseSuccess(
        "func call redirect stderr to stdout",
        "(func call redirect stderr to stdout)");
    ParseSuccess(
        "func call redirect to here", "(func call redirect stdout to here)");
    ParseSuccess(
        "func call redirect to ./output append",
        "(func call redirect stdout to ./output append)");
    ParseSuccess(
        "func call redirect stderr to stdout, stdin from input_file",
        "(func call redirect stderr to stdout, stdin from input_file)");
    ParseSuccess(
        "func call redirect to ./output, from ./input",
        "(func call redirect stdout to ./output, stdin from ./input)");
    ParseSuccess(
        "! cmd; redirect stdout to stderr, stderr to stdout | ! with pipe",
        "(! cmd; redirect stdout to stderr, stderr to stdout | ! with pipe;)");
}