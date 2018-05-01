#include "./util.h"

using namespace sushi::lexer::test;

TEST(SingleTokenTest, TestIdentifier) {
    NoIndentStrIsToks("i", TD(kIdent, "i"));
    NoIndentStrIsToks("_hi", TD(kIdent, "_hi"));
    NoIndentStrIsToks("_123", TD(kIdent, "_123"));
    NoIndentStrIsToks("bla    ", TD(kIdent, "bla"));
}

TEST(SingleTokenTest, TestInteger) {
    NoIndentStrIsToks("0", TD(kIntLit, 0));
    NoIndentStrIsToks("123", TD(kIntLit, 123));
    NoIndentStrIsToks("000001", TD(kIntLit, 1));
    NoIndentStrIsToks("1234   ", TD(kIntLit, 1234));
}

TEST(SingleTokenTest, TestRawToken) {
    RawStrIsTokens("1", TD(kRawString, "1"));
    RawStrIsTokens("a", TD(kRawString, "a"));
    RawStrIsTokens("?", TD(kRawString, "?"));
    RawStrIsTokens("\\n", TD(kRawString, "n"));
    RawStrIsTokens("\\;", TD(kRawString, ";"));
    RawStrIsTokens("\\;\\?", TD(kRawString, ";?"));
    RawStrIsTokens("\\ ", TD(kRawString, " "));
    RawStrIsTokens("h\\a\\ haha ", TD(kRawString, "ha haha"));
    RawStrIsTokens("h\\a\\ haha ", TD(kRawString, "ha haha"));
    std::string inter_test{'i', lexer::kInterDollar, lexer::kInterLBrace, 'i',
                           lexer::kInterRBrace};
    RawStrIsTokens(R"(i${i})", TD(kRawString, inter_test));
    RawStrIsTokens(R"(\$\{i\})", TD(kRawString, "${i}"));
}

TEST(SingleTokenTest, TestRawMode) {
    RawStrIsTokens("${", TD(kInterStart, 0));
    RawStrIsTokens("$id", TD(kIdent, "id"));
    RawStrIsTokens("$_id", TD(kIdent, "_id"));
}

TEST(SingleTokenTest, TestPath) {
    NoIndentStrIsToks(".", TD(kPathLit, "."));
    NoIndentStrIsToks("/", TD(kPathLit, "/"));
    NoIndentStrIsToks("..", TD(kPathLit, ".."));
    NoIndentStrIsToks("....", TD(kPathLit, "...."));
    NoIndentStrIsToks("~", TD(kPathLit, "~"));
    NoIndentStrIsToks("~/", TD(kPathLit, "~/"));
    NoIndentStrIsToks("../", TD(kPathLit, "../"));
    NoIndentStrIsToks("../hello/world", TD(kPathLit, "../hello/world"));
    NoIndentStrIsToks("~/hello/world", TD(kPathLit, "~/hello/world"));
    NoIndentStrIsToks("~/hello\\ world", TD(kPathLit, "~/hello world"));
    NoIndentStrIsToks("../    ", TD(kPathLit, "../"));
}

TEST(SingleTokenTest, TestPunctuation) {
    NoIndentStrIsToks("+", TK(kPlus));
    NoIndentStrIsToks("-", TK(kMinus));
    NoIndentStrIsToks("*", TK(kStar));
    NoIndentStrIsToks("//", TK(kDivide));
    NoIndentStrIsToks("%", TK(kPercent));
    NoIndentStrIsToks("<", TK(kLAngle));
    NoIndentStrIsToks(">", TK(kRAngle));
    NoIndentStrIsToks("<=", TK(kLessEq));
    NoIndentStrIsToks(">=", TK(kGreaterEq));
    NoIndentStrIsToks("==", TK(kDoubleEq));
    NoIndentStrIsToks("!=", TK(kNotEqual));
    NoIndentStrIsToks(",", TK(kComma));
    NoIndentStrIsToks("=", TK(kSingleEq));
    NoIndentStrIsToks(":", TK(kColon));
    NoIndentStrIsToks(";", TK(kSemicolon));
    NoIndentStrIsToks("[", TK(kLBracket));
    NoIndentStrIsToks("]", TK(kRBracket));
    NoIndentStrIsToks("{", TK(kLBrace));
    NoIndentStrIsToks("}", TK(kRBrace));
    NoIndentStrIsToks("(", TK(kLParen));
    NoIndentStrIsToks(")", TK(kRParen));
    NoIndentStrIsToks("!", TK(kExclamation));
    NoIndentStrIsToks("$", TK(kDollar));
}

TEST(SingleTokenTest, TestKeywords) {
    NoIndentStrIsToks("or", TK(kOr));
    NoIndentStrIsToks("and", TK(kAnd));
    NoIndentStrIsToks("define", TK(kDefine));
    NoIndentStrIsToks("return", TK(kReturn));
    NoIndentStrIsToks("export", TK(kExport));
    NoIndentStrIsToks("if", TK(kIf));
    NoIndentStrIsToks("else", TK(kElse));
    NoIndentStrIsToks("switch", TK(kSwitch));
    NoIndentStrIsToks("case", TK(kCase));
    NoIndentStrIsToks("default", TK(kDefault));
    NoIndentStrIsToks("for", TK(kFor));
    NoIndentStrIsToks("in", TK(kIn));
    NoIndentStrIsToks("break", TK(kBreak));
    NoIndentStrIsToks("continue", TK(kContinue));
    NoIndentStrIsToks("redirect", TK(kRedirect));
    NoIndentStrIsToks("from", TK(kFrom));
    NoIndentStrIsToks("to", TK(kTo));
    NoIndentStrIsToks("append", TK(kAppend));
    NoIndentStrIsToks("here", TK(kHere));
    NoIndentStrIsToks("Int", TK(kInt));
    NoIndentStrIsToks("Bool", TK(kBool));
    NoIndentStrIsToks("Unit", TK(kUnit));
    NoIndentStrIsToks("String", TK(kString));
    NoIndentStrIsToks("Char", TK(kChar));
    NoIndentStrIsToks("Path", TK(kPath));
    NoIndentStrIsToks("Array", TK(kArray));
    NoIndentStrIsToks("Map", TK(kMap));
    NoIndentStrIsToks("ExitCode", TK(kExitCode));
    NoIndentStrIsToks("FD", TK(kFd));
    NoIndentStrIsToks("true", TK(kTrue));
    NoIndentStrIsToks("false", TK(kFalse));
    NoIndentStrIsToks("unit", TK(kUnitLit));
    NoIndentStrIsToks("stdin", TK(kStdin));
    NoIndentStrIsToks("stdout", TK(kStdout));
    NoIndentStrIsToks("stderr", TK(kStderr));
}

TEST(SingleTokenTest, TestCharLiteral) {
    NoIndentStrIsToks("'a'", TD(kCharLit, 'a'));
    NoIndentStrIsToks("'\\n'", TD(kCharLit, '\n'));
    NoIndentStrIsToks("'\\''", TD(kCharLit, '\''));
    NoIndentStrIsToks("'\\\\'", TD(kCharLit, '\\'));
    NoIndentStrIsToks("'?'", TD(kCharLit, '?'));
    NoIndentStrIsToks("'\\!'", TD(kCharLit, '!'));
    NoIndentStrIsToks("'\\.'        ", TD(kCharLit, '.'));
}

TEST(SingleTokenTest, TestStringLiteral) {
    NoIndentStrIsToks(R"("")", TD(kStringLit, ""));
    NoIndentStrIsToks(R"("haha")", TD(kStringLit, "haha"));
    NoIndentStrIsToks(R"("ha\nha")", TD(kStringLit, "ha\nha"));
    NoIndentStrIsToks(R"("ha\tha")", TD(kStringLit, "ha\tha"));
    NoIndentStrIsToks(R"("say\"hello\"")", TD(kStringLit, "say\"hello\""));
    std::string inter_test{lexer::kInterDollar, lexer::kInterLBrace, 'i',
                           lexer::kInterRBrace};
    NoIndentStrIsToks(R"("${i}")", TD(kStringLit, inter_test));
    NoIndentStrIsToks(R"("\$\{i\}")", TD(kStringLit, "${i}"));
}