#include "./util.h"

using type::Error;

TEST(TypeCheckTest, TestIf) {
    TypingSuccess("if true: ()", {});
    TypingSuccess("if true: ()\nelse if false: ()", {});
    TypingSuccess("if (! hello): ()", {{"(! hello)", "ExitCode"}});
    TypingError("if 1: ()", Error::kInvalidType, "1");
}

TEST(TypeCheckTest, TestFor) {
    TypingSuccess("for true: ()", {});
    TypingSuccess("for 1 < 2: ()", {{"(1 < 2)", "Bool"}});
    TypingSuccess("for (! hello): ()", {{"(! hello)", "ExitCode"}});
    TypingError("for 1: ()", Error::kInvalidType, "1");
    TypingSuccess("for x in {1}: x", {{"{1}", "Array Int"}, {"x", "Int"}});
    TypingError("for x in 1: x", Error::kInvalidRange, "1");
    TypingError("for x in false: x", Error::kInvalidRange, "false");
}

TEST(TypeCheckTest, TestSwitch) {
    TypingSuccess("switch 1\ncase 1: ()\ncase 2: ()", {});
    TypingSuccess("switch 1\ncase 1: ()\ndefault: ()", {});
    TypingError(
        "switch 1\ncase (): ()\ndefault: ./h", Error::kInvalidType, "()");
}

TEST(TypeCheckTest, TestFunctionReturn) {
    TypingError("define f(): Int = return false", Error::kInvalidType, "false");
    TypingError("define f(): Int = return", Error::kMissingReturnValue, "");
    TypingSuccess("define f() = return;\nf", {{"f", "Function () ()"}});
    TypingSuccess("define f() = return; return\nf", {{"f", "Function () ()"}});
    TypingSuccess(
        "define f() = return 0; return 1\nf", {{"f", "Function Int ()"}});
    TypingError(
        "define f() =\n  return;\n  return false", Error::kInvalidType,
        "false");
    TypingSuccess(
        "define f(): Int = return f ()\nf", {{"f", "Function Int ()"}});
    TypingError("define f() = return f ()", Error::kMissingTypeDecl, "(f ())");
}

TEST(TypeCheckTest, TestAssignment) {
    TypingSuccess("define x = 1; x = 2", {{"x", "Int"}});
    TypingSuccess("define x = true; x = ! hello", {{"x", "Bool"}});
    TypingSuccess("define x = /hello; x = ./hello", {{"x", "Path"}});
    TypingSuccess("define x = {1}; x[0] = 2", {{"x", "Array Int"}});
    TypingSuccess("define x = {1}; x = {}", {{"x", "Array Int"}});
    TypingSuccess("define x = {'a': 1}; x['b'] = 2", {{"x", "Map Char Int"}});
    TypingSuccess(R"(define x = "hello"; x[0] = 'H')", {{"x", "String"}});
    TypingSuccess(
        R"(define x = {"hello"}; x[0][0] = 'H')", {{"x", "Array String"}});
    TypingError("1 = 2", Error::kInvalidLvalue, "1");
    TypingError("{1} = {2}", Error::kInvalidLvalue, "{1}");
}