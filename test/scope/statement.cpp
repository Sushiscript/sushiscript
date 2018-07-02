#include "util.h"

using namespace sushi::scope::test;

using ET = Error::Type;

TEST(StatementTest, TestVariableDef) {
    ScopeSuccess("define x = 0", {{"x"}}, {});
    ScopeSuccess("define x = 1; define y = 2", {{"x", "y"}}, {});

    ScopeError("define x = 0; define x = 1", ET::kRedefinedError);
}

TEST(StatementTest, TestFunctionDef) {
    ScopeSuccess("define foo (a : Int, b : String) : Bool = return a == 1",
        {{}, {"a", "b"}}, {1});

    ScopeSuccess(
R"(define a = 0
define foo (a : Int, b : String) : Bool =
    return a == 1)",
    {{"a"}, {"a", "b"}}, {1});

    ScopeError(
R"(define foo (a : Int) : Int =
    return 0
define foo (a : Int) : Int =
    return 1)",
        ET::kRedefinedError);
}

TEST(StatementTest, TestAssignment) {
    ScopeSuccess("define a = 0; a = 1", {{"a"}}, {0});
    ScopeError("a = 1", ET::kUndefinedError);
}

TEST(StatementTest, TestIfStmt) {
    ScopeSuccess("if true: define a = 0", {{}, {"a"}}, {});
    ScopeSuccess(R"(
if true:
    define a = 0
    a = 2
else:
    define a = 1
    a = 3)",
    {{}, {"a"}, {"a"}}, {1, 2});
    ScopeSuccess(R"(
define a = 0
if true:
    a = 1
    define a = 2
    a = 3
else:
    a = 4
    define b = 3
    b = 5)",
    {{"a"}, {"a"}, {"b"}}, {1, 1, 2, 2});

    ScopeError(R"(
if true:
    define a = 0
    define a = 1)",ET::kRedefinedError);
    ScopeError(R"(
if a:
    define b = 0
    )", ET::kUndefinedError);
}

TEST(StatementTest, TestReturnStmt) {
    ScopeSuccess("return 1", {{}}, {});
    ScopeSuccess("define a = 0; return a", {{"a"}}, {0});
    ScopeSuccess(R"(
define foo (a : Int) : Bool =
    define b = true
    return b)", {{}, {"a", "b"}}, {1});

    ScopeError("return a", ET::kUndefinedError);
}

TEST(StatementTest, TestSwitchStmt) {
    ScopeSuccess(R"(
define foo (para : Int) : Bool =
    return false
define a = 2
switch a
case 1: a = 2
case 2: ()
case foo: ()
default: ())", {{"a"}, {"para"}, {}, {}, {}, {}}, {0, 2, 0});

    ScopeError(R"(
switch a
case 1: a = 1)", ET::kUndefinedError);

    ScopeError(R"(
switch 1
case 1
    define a = 0
    define a = 1)", ET::kRedefinedError);
}

TEST(StatementTest, TestForStmt) {
    ScopeSuccess(R"(
for true:
    define a = 0
    a = 1
    break)",{{}, {"a"}}, {1});

    ScopeSuccess(R"(
for i in [1, 2, 3]:
    define b = i)", {{}, {"i", "b"}}, {});

    ScopeError(R"(
for i in [1, 2, 3]:
    define i = 1)", ET::kRedefinedError);
}
