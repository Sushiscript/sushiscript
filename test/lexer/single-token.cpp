#include "./util.h"

using namespace sushi::lexer::test;

TEST(SingleTokenTest, TestIdentifier) {
    NoIndentStrIsToks("i", TK(kIdent, "i"));
    NoIndentStrIsToks("_hi", TK(kIdent, "_hi"));
    NoIndentStrIsToks("_123", TK(kIdent, "_123"));
    NoIndentStrIsToks("bla    ", TK(kIdent, "bla"));
}

TEST(SingleTokenTest, TestInteger) {
    NoIndentStrIsToks("0", TK(kIntLit, 0));
    NoIndentStrIsToks("123", TK(kIntLit, 123));
    NoIndentStrIsToks("000001", TK(kIntLit, 1));
    NoIndentStrIsToks("1234   ", TK(kIntLit, 1234));
}


TEST(SingleTokenTest, TestRawToken) {
    RawStrIsTokens("1", TK(kRawString, "1"));
    RawStrIsTokens("a", TK(kRawString, "a"));
    RawStrIsTokens("?", TK(kRawString, "?"));
    RawStrIsTokens("\\n", TK(kRawString, "n"));
    RawStrIsTokens("\\;", TK(kRawString, ";"));
    RawStrIsTokens("\\;\\?", TK(kRawString, ";?"));
    RawStrIsTokens("\\ ", TK(kRawString, " "));
}

TEST(SingleTokenTest, TestPath) {
    NoIndentStrIsToks(".", TK(kPathLit, "."));
    NoIndentStrIsToks("/", TK(kPathLit, "/"));
    NoIndentStrIsToks("..", TK(kPathLit, ".."));
    NoIndentStrIsToks("....", TK(kPathLit, "...."));
    NoIndentStrIsToks("~", TK(kPathLit, "~"));
    NoIndentStrIsToks("~/", TK(kPathLit, "~/"));
    NoIndentStrIsToks("../", TK(kPathLit, "../"));
    NoIndentStrIsToks("../hello/world", TK(kPathLit, "../hello/world"));
    NoIndentStrIsToks("~/hello/world", TK(kPathLit, "~/hello/world"));
    NoIndentStrIsToks("../    ", TK(kPathLit, "../"));
}

TEST(SingleTokenTest, TestPunctuation) {
    NoIndentStrIsToks("+", TK(kPlus, "+"));
    NoIndentStrIsToks("-", TK(kMinus, "-"));
    NoIndentStrIsToks("*", TK(kStar, "*"));
    NoIndentStrIsToks("//", TK(kDivide, "//"));
    NoIndentStrIsToks("%", TK(kPercent, "%"));
    NoIndentStrIsToks("<", TK(kLAngle, "<"));
    NoIndentStrIsToks(">", TK(kRAngle, ">"));
    NoIndentStrIsToks("<=", TK(kLessEq, "<="));
    NoIndentStrIsToks(">=", TK(kGreaterEq, ">="));
    NoIndentStrIsToks("==", TK(kDoubleEq, "=="));
    NoIndentStrIsToks("!=", TK(kNotEqual, "!="));
    NoIndentStrIsToks(",", TK(kComma, ","));
    NoIndentStrIsToks("=", TK(kSingleEq, "="));
    NoIndentStrIsToks(":", TK(kColon, ":"));
    NoIndentStrIsToks(";", TK(kSemicolon, ";"));
    NoIndentStrIsToks("[", TK(kLBracket, "["));
    NoIndentStrIsToks("]", TK(kRBracket, "]"));
    NoIndentStrIsToks("{", TK(kLBrace, "{"));
    NoIndentStrIsToks("}", TK(kRBrace, "}"));
    NoIndentStrIsToks("(", TK(kLParen, "("));
    NoIndentStrIsToks(")", TK(kRParen, ")"));
    NoIndentStrIsToks("!", TK(kExclamation, "!"));
    NoIndentStrIsToks("$", TK(kDollar, "$"));
}

TEST(SingleTokenTest, TestKeywords) {
    NoIndentStrIsToks("or", TK(kOr, 0));
    NoIndentStrIsToks("and", TK(kAnd, 0));
    NoIndentStrIsToks("define", TK(kDefine, 0));
    NoIndentStrIsToks("return", TK(kReturn, 0));
    NoIndentStrIsToks("export", TK(kExport, 0));
    NoIndentStrIsToks("if", TK(kIf, 0));
    NoIndentStrIsToks("else", TK(kElse, 0));
    NoIndentStrIsToks("switch", TK(kSwitch, 0));
    NoIndentStrIsToks("case", TK(kCase, 0));
    NoIndentStrIsToks("default", TK(kDefault, 0));
    NoIndentStrIsToks("for", TK(kFor, 0));
    NoIndentStrIsToks("in", TK(kIn, 0));
    NoIndentStrIsToks("break", TK(kBreak, 0));
    NoIndentStrIsToks("continue", TK(kContinue, 0));
    NoIndentStrIsToks("redirect", TK(kRedirect, 0));
    NoIndentStrIsToks("from", TK(kFrom, 0));
    NoIndentStrIsToks("to", TK(kTo, 0));
    NoIndentStrIsToks("append", TK(kAppend, 0));
    NoIndentStrIsToks("here", TK(kHere, 0));
    NoIndentStrIsToks("Int", TK(kInt, 0));
    NoIndentStrIsToks("Bool", TK(kBool, 0));
    NoIndentStrIsToks("Unit", TK(kUnit, 0));
    NoIndentStrIsToks("String", TK(kString, 0));
    NoIndentStrIsToks("Char", TK(kChar, 0));
    NoIndentStrIsToks("Path", TK(kPath, 0));
    NoIndentStrIsToks("Array", TK(kArray, 0));
    NoIndentStrIsToks("Map", TK(kMap, 0));
    NoIndentStrIsToks("ExitCode", TK(kExitCode, 0));
    NoIndentStrIsToks("FD", TK(kFd, 0));
    NoIndentStrIsToks("true", TK(kTrue, 0));
    NoIndentStrIsToks("false", TK(kFalse, 0));
    NoIndentStrIsToks("unit", TK(kUnitLit, 0));
    NoIndentStrIsToks("stdin", TK(kStdin, 0));
    NoIndentStrIsToks("stdout", TK(kStdout, 0));
    NoIndentStrIsToks("stderr", TK(kStderr, 0));
}