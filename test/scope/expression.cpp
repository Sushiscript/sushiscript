#include "util.h"

using namespace sushi::scope::test;

using ET = Error::Type;

TEST(ExpressionTest, TestVariable) {
    ScopeSuccess(R"(
define a = 0
a = 1
if true:
    a = 2
    if true:
        a = 3
else:
    a = 4
a = 5)", {{"a"}, {}, {}, {}}, {0, 1, 2, 3, 0});

    ScopeError("a", ET::kUndefinedError);
}

TEST(ExpressionTest, Literal) {
    // String
    ScopeSuccess("\"abc\"", {{}}, {});
    ScopeSuccess(R"(
define a : String = "abc"
a = "${a}${a}")", {{"a"}}, {0, 0, 0});

    ScopeError("\"${a}${a}\"", ET::kUndefinedError);

    // Path
    ScopeSuccess("/path/to/sth", {{}}, {});
    ScopeSuccess("define a = /path/to/sth\na = /${a}${a}", {{"a"}}, {0, 0, 0});
    ScopeError("/path${a}", ET::kUndefinedError);

    // RelPath
    ScopeSuccess("./path/to/sth", {{}}, {});
    ScopeSuccess(R"(define a = ./path/to/sth; a = ./${a}${a})", {{"a"}}, {0, 0, 0});
    ScopeError("./path${a}", ET::kUndefinedError);

    // Array
    ScopeSuccess("{1, 2, 3}", {{}}, {});
    ScopeSuccess(R"(
define a = 1
define b = 2
{a, b, 3}
if true:
    {a, b, 3})", {{"a", "b"}, {}}, {0, 0, 1, 1});
    ScopeError("{a, 2}", ET::kUndefinedError);

    // Map
    ScopeSuccess(R"({"a": 1, "b": 2})", {{}}, {});
    ScopeSuccess(R"(
define a_k = "a"
define a_v = 1
define b_k = "b"
define b_v = 2
{a_k: a_v, b_k: b_v})", {{"a_k", "a_v", "b_k", "b_v"}}, {0, 0, 0, 0});
    ScopeError("{a_k: a_v}", ET::kUndefinedError);
}

TEST(ExpressionTest, TestUnaryExpr) {
    ScopeSuccess("not false", {{}}, {});
    ScopeSuccess("define a = false; a = not a", {{"a"}}, {0, 0});
    ScopeSuccess("define a = 1; a = -a", {{"a"}}, {0, 0});
    ScopeSuccess("define a = 1; a = +a", {{"a"}}, {0, 0});

    ScopeError("not a", ET::kUndefinedError);
    ScopeError("-a", ET::kUndefinedError);
    ScopeError("+a", ET::kUndefinedError);
}

TEST(ExpressionTest, TestBinaryExpr) {
    ScopeSuccess("1 + 1", {{}}, {});
    constexpr char kDefineAB[] = "define a = 1\ndefine b = 0\n";
    #define BinarySuccess(binary_expr) \
        ScopeSuccess(std::string(kDefineAB) + binary_expr, {{"a", "b"}}, {0, 0})
    BinarySuccess("a + b");
    BinarySuccess("a - b");
    BinarySuccess("a * b");
    BinarySuccess("a // b");
    BinarySuccess("a % b");
    BinarySuccess("a < b");
    BinarySuccess("a > b");
    BinarySuccess("a <= b");
    BinarySuccess("a >= b");
    BinarySuccess("a == b");
    BinarySuccess("a != b");
    BinarySuccess("a and b");
    BinarySuccess("a or b");

    #define BinaryError(binary_expr) \
        ScopeError(binary_expr, ET::kUndefinedError)
    BinaryError("a + b");
    BinaryError("a - b");
    BinaryError("a * b");
    BinaryError("a // b");
    BinaryError("a % b");
    BinaryError("a < b");
    BinaryError("a > b");
    BinaryError("a <= b");
    BinaryError("a >= b");
    BinaryError("a == b");
    BinaryError("a != b");
    BinaryError("a and b");
    BinaryError("a or b");
}

TEST(ExpressionTest, TestFunctionCall) {
    ScopeSuccess("define func () : Int = return 1\nfunc", {{"func"}, {}}, {0});
    ScopeError("func", ET::kUndefinedError);
}

TEST(ExpressionTest, TestCommand) {
    ScopeSuccess("! echo \"hello\"", {{}}, {});
    ScopeSuccess("define ec = \"echo\"\n! ${ec} \"${ec}\"", {{"ec"}}, {0, 0});
    ScopeError("! echo \"${b}\"", ET::kUndefinedError);
}

TEST(ExpressionTest, TestIndexing) {
    ScopeSuccess("{1, 2, 3}[1]", {{}}, {});
    ScopeSuccess("define arr = {1, 2, 3}; define ind = 1; arr[ind]", {{"arr", "ind"}}, {0, 0});
    ScopeSuccess("define map = {\"a\": 1}; define key = \"a\"; map[key]", {{"map", "key"}}, {0, 0});
    ScopeError("define arr = {1, 2, 3}; arr[index]", ET::kUndefinedError);
    ScopeError("define key = \"a\"; map[key]", ET::kUndefinedError);
}
