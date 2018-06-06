#include "./util.h"

using lexer::Token;

TEST(ParseErrorTest, TestLexicalError) {
    ParseError("'aa'", PE(kLexicalError, TEL(kInvalidChar, 1, 1)));
    ParseError("1 + 'aa' 1", PE(kLexicalError, TEL(kInvalidChar, 1, 5)));
}

TEST(ParseErrorTest, TestExpectExpression) {
    ParseError("1 + ", PE(kExpectExpression, Token::Eof()));
}

TEST(ParseErrorTest, TestEmptyBlock) {
    ParseError("", PE(kEmptyBlock, Token::Eof()));
    ParseError("if a > 0", PE(kEmptyBlock, Token::Eof()));
    ParseError("if a > 0\nprint a", PE(kEmptyBlock, TDL(kIndent, 0, 2, 1)));
    ParseError("for a > 0", PE(kEmptyBlock, Token::Eof()));
    ParseError("for a > 0\nprint a", PE(kEmptyBlock, TDL(kIndent, 0, 2, 1)));
    ParseError("define f() =", PE(kEmptyBlock, Token::Eof()));
    ParseError("define f() =\nprint a", PE(kEmptyBlock, TDL(kIndent, 0, 2, 1)));
}

TEST(ParseErrorTest, TestUnexpectIndent) {
    ParseError("1 + 1\n 1 + 1", PE(kUnexpectIndent, TDL(kIndent, 1, 2, 1)));
    ParseError(" 1 + 1\n  1 + 1", PE(kUnexpectIndent, TDL(kIndent, 2, 2, 1)));
    ParseError(
        " 1 + 1\n  1 + 1\n   1 + 1", PE(kUnexpectIndent, TDL(kIndent, 2, 2, 1)),
        PE(kUnexpectIndent, TDL(kIndent, 3, 3, 1)));
    ParseError(
        "1 + 1\n  1 + 1\n 1 + 1", PE(kUnexpectIndent, TDL(kIndent, 2, 2, 1)),
        PE(kUnexpectIndent, TDL(kIndent, 1, 3, 1)));
    ParseError(
        "1 + 1\n  1 + 1\n 1 + 1\n  1 + 1",
        PE(kUnexpectIndent, TDL(kIndent, 2, 2, 1)),
        PE(kUnexpectIndent, TDL(kIndent, 1, 3, 1)),
        PE(kUnexpectIndent, TDL(kIndent, 2, 4, 1)));
}

TEST(ParseErrorTest, TestExpectStatementEnd) {
    ParseError("define a = 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 14)));
    ParseError(
        "define a = 1 + 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 18)));
    ParseError("return 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 10)));
    ParseError("return 1 + 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 14)));
    ParseError("a = 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 7)));
    ParseError("a = 1 + 1 ()", PE(kExpectStatementEnd, TL(kUnit, 1, 11)));
}

TEST(ParseErrorTest, TestExpectCommand) {
    ParseError("func call | ", PE(kExpectCommand, Token::Eof()));
    ParseError("func call | ()", PE(kExpectCommand, TL(kUnit, 1, 13)));
}

TEST(ParseErrorTest, TestExpectType) {
    ParseError("define a: = 1", PE(kExpectType, TL(kSingleEq, 1, 11)));
    ParseError("define f(): = 1", PE(kExpectType, TL(kSingleEq, 1, 13)));
    ParseError("define f(a:) = 1", PE(kExpectType, TL(kRParen, 1, 12)));
    ParseError("define f(a) = 1", PE(kExpectType, TL(kRParen, 1, 11)));
}

TEST(ParseErrorTest, TestExpectSimpleType) {
    ParseError(
        "define a: Map = {}", PE(kExpectSimpleType, TL(kSingleEq, 1, 15)));
    ParseError(
        "define a: Array = {}", PE(kExpectSimpleType, TL(kSingleEq, 1, 17)));
    ParseError(
        "define a: Map Array = {}", PE(kExpectSimpleType, TL(kArray, 1, 15)));
    ParseError(
        "define a: Map Int Array = {}",
        PE(kExpectSimpleType, TL(kArray, 1, 19)));
}

TEST(ParseErrorTest, TestWrongTypeKind) {
    ParseError(
        "define a: Function = ()", PE(kWrongTypeKind, TL(kFunction, 1, 11)));
}

TEST(ParseErrorTest, TestExpectMappedValue) {
    ParseError("{1: 2, ()}", PE(kExpectMappedValue, TL(kUnit, 1, 8)));
    ParseError("{1: 2, (), 4 : 5}", PE(kExpectMappedValue, TL(kUnit, 1, 8)));
    ParseError(
        "{1: 2, 3, 4}", PE(kExpectMappedValue, TDL(kIntLit, 3, 1, 8)),
        PE(kExpectMappedValue, TDL(kIntLit, 4, 1, 11)));
}

TEST(ParseErrorTest, TestInvalidLoopLevel) {
    ParseError("break", PE(kInvalidLoopLevel, TL(kBreak, 1, 1)));
    ParseError("break 1", PE(kInvalidLoopLevel, TL(kBreak, 1, 1)));
    ParseError(
        "for i in {1, 2, 3}: break 2",
        PE(kInvalidLoopLevel, TL(kBreak, 1, 21)));
    ParseError(
        "for i in {1, 2, 3}: for a < 0: break 3",
        PE(kInvalidLoopLevel, TL(kBreak, 1, 32)));
}

TEST(ParseErrorTest, TestMatchBracket) {
    ParseError("a[1\n1 + 2", PE(kExpectToken, TL(kRBracket, 1, 4)));
    ParseError("a[1 1 2] + 1 + 2", PE(kExpectToken, TL(kRBracket, 1, 5)));
    ParseError("1 + (1 + 2", PE(kExpectToken, TL(kRParen, -1, -1)));
    ParseError("(1 1) + 2", PE(kExpectToken, TL(kRParen, 1, 4)));
    ParseError("\"${1 + 1 2}\"", PE(kExpectToken, TL(kRBrace, 1, 10)));
}