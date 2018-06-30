#include "./util.h"

using type::Error;

TEST(TypeCheckTest, TestVariables) {
    TypingSuccess("define a: Int = 1; a", {{"a", "Int"}});
    TypingSuccess("define a = 1; a", {{"a", "Int"}});
    TypingSuccess("define a: Map Int Int = {}; a", {{"a", "Map Int Int"}});
}

TEST(TypeCheckTest, TestFunctions) {
    TypingSuccess(
        "define func(): Int = return 1\nfunc", {{"func", "Function Int Unit"}});
    TypingSuccess(
        "define func() = return 1\nfunc", {{"func", "Function Int Unit"}});
    TypingSuccess(
        "define func() = return 1\nfunc\ndefine a = func(); a",
        {{"func", "Function Int Unit"}, {"(func ())", "Int"}, {"a", "Int"}});
    TypingError("define x = 1; x ()", Error::kInvalidFunction, "x");
    TypingError(
        "define f() = return 1; f () ()", Error::kWrongNumOfParams,
        "(f () ())");
    TypingError(
        "define f(x: Int, y: Int) = return 1; f 1", Error::kWrongNumOfParams,
        "(f 1)");
}

TEST(TypeCheckTest, TestSimpleLiteral) {
    TypingSuccess("1", {{"1", "Int"}});
    TypingSuccess("'c'", {{"'c'", "Char"}});
    TypingSuccess("()", {{"()", "()"}});
    TypingSuccess(R"("hello")", {{R"("hello")", "String"}});
    TypingSuccess("./hello", {{"./hello", "RelPath"}});
    TypingSuccess("/hello", {{"/hello", "Path"}});
    TypingSuccess("true", {{"true", "Bool"}});
    TypingSuccess("false", {{"false", "Bool"}});
    TypingSuccess("stdin", {{"stdin", "Fd"}});
    TypingSuccess("stdout", {{"stdout", "Fd"}});
    TypingSuccess("stderr", {{"stderr", "Fd"}});
}

TEST(TypeCheckTest, TestImplicitConversion) {
    TypingSuccess("define a: Path = ./hello; a", {{"a", "Path"}});
}

TEST(TypeCheckTest, TestArrayLiteral) {
    TypingSuccess("{1}", {{"{1}", "Array Int"}});
    TypingSuccess("{1, 2, 3}", {{"{1, 2, 3}", "Array Int"}});
    TypingSuccess("{/hello, ./hi}", {{"{/hello, ./hi}", "Array Path"}});
    TypingSuccess("define a: Array Int = {}; a", {{"a", "Array Int"}});
}

TEST(TypeCheckTest, TestMapLiteral) {
    TypingSuccess("{1: 2}", {{"{1: 2}", "Map Int Int"}});
    TypingSuccess("{1: 2, 3: 4}", {{"{1: 2}", "Map Int Int"}});
    TypingSuccess(
        "{/hello : /hello, ./hi : ./hi}",
        {{"{/hello: /hello, ./hi: ./hi}", "Map Path Path"}});
}

TEST(TypeCheckTest, TestCommand) {
    TypingSuccess("!hello world", {{"(! hello world)", "ExitCode"}});
    TypingSuccess(
        "!hello world, redirect to here",
        {{"(! hello world redirect to here)", "String"}});
}
TEST(TypeCheckTest, TestRedirection) {
    TypingSuccess("! hello, redirect from ./hello", {{"./hello", "RelPath"}});
    TypingSuccess("! hello, redirect from /hello", {{"/hello", "Path"}});
    TypingSuccess("! hello, redirect to /hello", {{"/hello", "Path"}});
    TypingSuccess("! hello, redirect to ./hello", {{"/hello", "RelPath"}});
    TypingSuccess("! hello, redirect to stderr", {{"stderr", "Fd"}});
}
TEST(TypeCheckTest, TestPipe) {
    TypingSuccess(
        "!hello world | !hello world",
        {{"(! hello world | ! hello world)", "ExitCode"}});
    TypingSuccess(
        "define func(): Int = return 1\n!hello world | func ()",
        {{"func", "Function Int Unit"},
         {"(func ())", "Int"},
         {"(! hello world | func ())", "Int"}});
    TypingSuccess(
        "define func(): Int = return 1\n!hello world | func () redirect to "
        "here",
        {{"(! hello world | func () redirect to here)", "String"}});
}

TEST(TypeCheckTest, TestIndexing) {
    TypingSuccess("{true}[0]", {{"{1}", "Array Bool"}, {"{true}[0]", "Bool"}});
    TypingError("{true}['a']", Error::kInvalidType, "'a'");
    TypingError("{true}[{0}]", Error::kInvalidType, "{0}");
    TypingSuccess(
        R"("hello"[0])", {{R"("hello")", "String"}, {R"("hello"[0])", "Char"}});
    TypingSuccess(
        "{1: true}[1]",
        {{"{1: true}", "Map Int Bool"}, {"{1: true}[1]", "Bool"}});
    TypingError("{1: true}[false]", Error::kInvalidType, "false");
    TypingError("{1: true}[()]", Error::kInvalidType, "()");
    TypingSuccess(
        "{true: 1}[true]",
        {{"{true: 1}", "Map Bool Int"}, {"{true: 1}[true]", "Int"}});
    TypingSuccess(
        R"({true: "hello"}[true][0])",
        {{R"({true: "hello"})", "Map Bool String"},
         {R"({true: "hello"}[true])", "String"},
         {R"({true: "hello"}[true][0])", "Char"}});
    TypingSuccess(
        R"({"hello"}[0][0])", {{R"({"hello"})", "Array String"},
                               {R"({"hello"}[0])", "String"},
                               {R"({"hello"}[0][0])", "Char"}});
    TypingError("1[0]", Error::kInvalidIndexable, "1");
    TypingError("()[0]", Error::kInvalidIndexable, "()");
}

TEST(TypeCheckTest, TestUnaryOperator) {
    TypingSuccess("-1", {{"(- 1)", "Int"}});
    TypingSuccess("+1", {{"(+ 1)", "Int"}});
    TypingSuccess("not true", {{"(not true)", "Bool"}});
    TypingSuccess("not false", {{"(not false)", "Bool"}});
}

TEST(TypeCheckTest, TestCollectionConcat) {
    TypingSuccess(R"(define a = {1} + {2}; a)", {{"a", "Array Int"}});
    TypingSuccess(R"(define a = {1: 1} + {2: 2}; a)", {{"a", "Map Int Int"}});
}

TEST(TypeCheckTest, TestStringOp) {
    TypingSuccess(R"(define a = "" + ""; a)", {{"a", "String"}});
    TypingSuccess(R"(define a = "" * 4; a)", {{"a", "String"}});
}

TEST(TypeCheckTest, TestArithmetic) {
    TypingSuccess("define a = 1 + 1; a", {{"a", "Int"}});
    TypingSuccess("define a = 1 - 1; a", {{"a", "Int"}});
    TypingSuccess("define a = 1 * 1; a", {{"a", "Int"}});
    TypingSuccess("define a = 1 / 1; a", {{"a", "Int"}});
    TypingSuccess("define a = 1 % 1; a", {{"a", "Int"}});
}

TEST(TypeCheckTest, TestPathConcat) {
    TypingSuccess(R"(define a = /hello // ./world; a)", {{"a", "Path"}});
    TypingSuccess(R"(define a = ./hello // ./world; a)", {{"a", "RelPath"}});
}

TEST(TypeCheckTest, TestEqual) {
    TypingSuccess("define a = 1 == 1; a", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' == 'a'; a", {{"a", "Bool"}});
    TypingSuccess("define a = {1} == {1}; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "abc" == "bcd"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = ./hi == ./hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = /hi == /hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = /hi == ./hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = true == false; a", {{"a", "Bool"}});
    TypingSuccess("define a = () == (); a", {{"a", "Bool"}});
    TypingSuccess("define a = stdin == stdout; a", {{"a", "Bool"}});
    TypingSuccess("define a = 1 != 1; a", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' != 'a'; a", {{"a", "Bool"}});
    TypingSuccess("define a = {1} != {1}; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "abc" != "bcd"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = ./hi != ./hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = /hi != /hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = /hi != ./hi; a", {{"a", "Bool"}});
    TypingSuccess("define a = true != false; a", {{"a", "Bool"}});
    TypingSuccess("define a = () != (); a", {{"a", "Bool"}});
    TypingSuccess("define a = stdin != stdout; a", {{"a", "Bool"}});
}

TEST(TypeCheckTest, TestOrder) {
    TypingSuccess("define a = 1 < 2; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "hi" < "wow"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' < 'b'; a", {{"a", "Bool"}});
    TypingSuccess("define a = 1 <= 2; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "hi" <= "wow"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' <= 'b'; a", {{"a", "Bool"}});
    TypingSuccess("define a = 1 >= 2; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "hi" >= "wow"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' >= 'b'; a", {{"a", "Bool"}});
    TypingSuccess("define a = 1 > 2; a", {{"a", "Bool"}});
    TypingSuccess(R"(define a = "hi" > "wow"; a)", {{"a", "Bool"}});
    TypingSuccess("define a = 'a' > 'b'; a", {{"a", "Bool"}});
}

TEST(TypeCheckTest, TestLogic) {
    TypingSuccess("define a = true and false; a", {{"a", "Bool"}});
    TypingSuccess("define a = true or false; a", {{"a", "Bool"}});
}