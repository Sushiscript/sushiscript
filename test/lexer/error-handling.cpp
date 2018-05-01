#include "./util.h"

using namespace sushi::lexer::test;
using sushi::lexer::Error;

TEST(ErrorHandling, TestUnknownChar) {
    NoIndentStrIsToks("\tident", TD(kOtherChar, '\t'), TD(kIdent, "ident"));
    StringIsTokens(
        "  \t  ident", TD(kIndent, 2), TD(kOtherChar, '\t'),
        TD(kIdent, "ident"));
    NoIndentStrIsToks(
        "ide\tnt", TD(kIdent, "ide"), TD(kOtherChar, '\t'), TD(kIdent, "nt"));
}

TEST(ErrorHandling, TestUnclosedString) {
    NoIndentStrIsToks<true>(
        R"("unclosed string)", TEL(kUnclosedStringQuote, 1, 1));
    NoIndentStrIsToks<true>(
        "\"cannot close in next line\n\"", TEL(kUnclosedStringQuote, 1, 1),
        TK(kLineBreak), TDL(kIndent, 0, 2, 1), TEL(kUnclosedStringQuote, 2, 1));
    NoIndentStrIsToks<true>(
        "\"error\nrecovery", TEL(kUnclosedStringQuote, 1, 1), TK(kLineBreak),
        TDL(kIndent, 0, 2, 1), TDL(kIdent, "recovery", 2, 1));
}

TEST(ErrorHandling, TestUnclosedChar) {
    NoIndentStrIsToks<true>("'", TEL(kUnclosedCharQuote, 1, 1));
    NoIndentStrIsToks<true>("'unclosed", TEL(kUnclosedCharQuote, 1, 1));
    NoIndentStrIsToks<true>(
        "haha 'u", TDL(kIdent, "haha", 1, 1), TEL(kUnclosedCharQuote, 1, 6));
}

TEST(ErrorHandling, TestInvalidChar) {
    NoIndentStrIsToks<true>(
        "'many char' ident", TEL(kInvalidChar, 1, 1),
        TDL(kIdent, "ident", 1, 13));
    NoIndentStrIsToks<true>(
        "'' ident", TEL(kInvalidChar, 1, 1), TDL(kIdent, "ident", 1, 4));
}

TEST(ErrorHandling, TestExpectSlash) {
    NoIndentStrIsToks<true>("..hahaha", TEL(kPathExpectSlash, 1, 3));
    NoIndentStrIsToks<true>("~hahaha", TEL(kPathExpectSlash, 1, 2));
    NoIndentStrIsToks<true>("~~hahaha", TEL(kPathExpectSlash, 1, 2));
    NoIndentStrIsToks<true>(".hahaha", TEL(kPathExpectSlash, 1, 2));
}

TEST(ErrorHandling, TestSingleDollar) {
    RawStrIsTokens<true>("$", TEL(kSingleDollar, 1, 1));
    RawStrIsTokens<true>(
        "$ ident", TEL(kSingleDollar, 1, 1), TDL(kRawString, "ident", 1, 3));
}