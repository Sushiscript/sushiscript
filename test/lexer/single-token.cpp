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
    NoIndentStrIsToks("|", TK(kPipe));
    NoIndentStrIsToks("$", TK(kDollar));
}

TEST(SingleTokenTest, TestKeywords) {
    NoIndentStrIsToks("()", TK(kUnit));
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
    NoIndentStrIsToks("String", TK(kString));
    NoIndentStrIsToks("Char", TK(kChar));
    NoIndentStrIsToks("Path", TK(kPath));
    NoIndentStrIsToks("RelPath", TK(kRelPath));
    NoIndentStrIsToks("Array", TK(kArray));
    NoIndentStrIsToks("Map", TK(kMap));
    NoIndentStrIsToks("ExitCode", TK(kExitCode));
    NoIndentStrIsToks("FD", TK(kFd));
    NoIndentStrIsToks("true", TK(kTrue));
    NoIndentStrIsToks("false", TK(kFalse));
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
