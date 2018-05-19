#include "./util.h"

TEST(StatementTest, TestAssignment) {
    ParseSuccess("x = 1", "x = 1");
    ParseSuccess("x[1] = \"hello\"", "x[1] = \"hello\"");
    ParseSuccess(
        R"(x =
        "a very long expression")",
        "x = \"a very long expression\"");
}

TEST(StatementTest, TestVariableDef) {
    ParseSuccess("define x = 1", "define x = 1");
    ParseSuccess("define x: Int = 1", "define x: Int = 1");
    ParseSuccess("export define x: Int = 1", "export define x: Int = 1");
    ParseSuccess("define x: Int =\n   1", "define x: Int = 1");
}

TEST(StatementTest, TestType) {
    ParseSuccess("define x: () = ()", "define x: () = ()");
    ParseSuccess("define x: Array Int = ()", "define x: (Array Int) = ()");
    ParseSuccess("define x: Map Int Int = ()", "define x: (Map Int Int) = ()");
    ParseSuccess(
        "define x: Function Int = ()", "define x: (Function Int) = ()");
    ParseSuccess(
        "define x: Function Int Int = ()", "define x: (Function Int Int) = ()");
    ParseSuccess(
        "define x: Function (Array Int) = ()",
        "define x: (Function (Array Int)) = ()");
    ParseSuccess(
        "define x: Function Int Int (Array Int) = ()",
        "define x: (Function Int Int (Array Int)) = ()");
    ParseSuccess(
        "define x: Function (Map Int Int) Int = ()",
        "define x: (Function (Map Int Int) Int) = ()");
}

TEST(StatementTest, TestReturn) {
    ParseSuccess("return", "return");
    ParseSuccess("return ()", "return ()");
    ParseSuccess("return 1 + 1", "return (1 + 1)");
    ParseSuccess("return 1 + 1;", "return (1 + 1)");
}

TEST(StatementTest, TestFor) {
    ParseSuccess("for i < 4: i = i + 1", "for (i < 4)\n  i = (i + 1)");
    ParseSuccess(
        "for i in {1, 2, 3}: print i", "for i in {1, 2, 3}\n  (print i)");
    ParseSuccess(
        "for i in {1, 2, 3}\n print i", "for i in {1, 2, 3}\n  (print i)");
    ParseSuccess(
        "for i in {1, 2, 3}:\n print i", "for i in {1, 2, 3}\n  (print i)");
    ParseSuccess(
        "for i in {1, 2, 3}: for j in {2, 3, 4}: print i j",
        "for i in {1, 2, 3}\n  for j in {2, 3, 4}\n    (print i j)");
    ParseSuccess(
        "for i in {1, 2, 3}\n for j in {2, 3, 4}\n  print i j\n  print j i",
        "for i in {1, 2, 3}\n  for j in {2, 3, 4}\n    (print i j)\n    (print "
        "j i)");
    ParseSuccess(
        "for i in {1, 2, 3}\n for j in {2, 3, 4}\n  print i j\n  print j i\n "
        "print i i",
        "for i in {1, 2, 3}\n  for j in {2, 3, 4}\n    (print i j)\n    (print "
        "j i)\n  (print i i)");
}

TEST(StatementTest, TestLoopControl) {
    ParseSuccess("for i < 4: continue", "for (i < 4)\n  continue");
    ParseSuccess("for i < 4: continue 1", "for (i < 4)\n  continue");
    ParseSuccess(
        "for i < 4: for j < 5: continue 2",
        "for (i < 4)\n  for (j < 5)\n    continue 2");
    ParseSuccess("for i < 4: break", "for (i < 4)\n  break");
    ParseSuccess("for i < 4: break 1", "for (i < 4)\n  break");
    ParseSuccess(
        "for i < 4: for j < 5: break 2",
        "for (i < 4)\n  for (j < 5)\n    break 2");
}

TEST(StatementTest, TestIf) {
    ParseSuccess("if x < 5: print x", "if (x < 5)\n  (print x)");
    ParseSuccess("if x < 5\n print x", "if (x < 5)\n  (print x)");
    ParseSuccess("if x < 5:\n print x", "if (x < 5)\n  (print x)");
    ParseSuccess(
        "if x < 5:\n print x\nelse: print y",
        "if (x < 5)\n  (print x)\nelse\n  (print y)");
    ParseSuccess(
        "if x < 5:\n print x\nelse\n print y",
        "if (x < 5)\n  (print x)\nelse\n  (print y)");
    ParseSuccess(
        "if x < 5:\n print x\nelse:\n print y",
        "if (x < 5)\n  (print x)\nelse\n  (print y)");
    ParseSuccess(
        "if x < 5:\n print x\nelse if x < 10:\n print y",
        "if (x < 5)\n  (print x)\nelse\n  if (x < 10)\n    (print y)");
    ParseSuccess(
        "if x < 5:\n print x\nelse if x < 10:\n print y\nelse\n print z",
        "if (x < 5)\n  (print x)\nelse\n  if (x < 10)\n    (print "
        "y)\n  else\n    (print z)");
    ParseSuccess(
        "if x < 5:\n if x < 10:\n  print x\nelse\n print y",
        "if (x < 5)\n  if (x < 10)\n    (print x)\nelse\n  (print y)");
    ParseSuccess(
        "if x < 5:\n if x < 10:\n  print x\n print y\nelse\n print z",
        "if (x < 5)\n  if (x < 10)\n    (print x)\n  (print y)\nelse\n  (print "
        "z)");
}

TEST(StatementTest, TestSwitch) {
    ParseSuccess(
        "switch x\n case 1\n  print x\n case 2\n  print x\n default\n  print y",
        "switch x\n case 1\n  (print x)\n case 2\n  (print x)\n default\n  "
        "(print y)");
    ParseSuccess(
        "switch ! cmd\n case 1\n  print x\n case 2\n  print x\n default\n  "
        "print y",
        "switch (! cmd)\n case 1\n  (print x)\n case 2\n  (print x)\n "
        "default\n  "
        "(print y)");
    ParseSuccess(
        "switch ! cmd\n case ! cmd2\n  print x\n case 2\n  print x\n default\n  "
        "print y",
        "switch (! cmd)\n case (! cmd2)\n  (print x)\n case 2\n  (print x)\n "
        "default\n  "
        "(print y)");
    ParseSuccess(
        "switch x\n case 1\n case 2\n  print x\n default\n  print y",
        "switch x\n case 1\n\n case 2\n  (print x)\n default\n  (print y)");
}

TEST(StatementTest, TestFunctionDef) {
    ParseSuccess("define f() = return 1", "define f() =\n  return 1");
    ParseSuccess("define f() =\n return 1", "define f() =\n  return 1");
    ParseSuccess(
        "export define f() = return 1", "export define f() =\n  return 1");
    ParseSuccess("define f( ) = return 1", "define f() =\n  return 1");
    ParseSuccess("define f(): Int = return 1", "define f(): Int =\n  return 1");
    ParseSuccess(
        "define f(x: Int): Int = return x",
        "define f(x: Int): Int =\n  return x");
    ParseSuccess(
        "define f(x: Int, y: String): Int = return length y",
        "define f(x: Int, y: String): Int =\n  return (length y)");
    ParseSuccess(
        "define f(x: Int, y: String, z: Array Int): Int =\n return z[x]",
        "define f(x: Int, y: String, z: (Array Int)): Int =\n  return z[x]");
}