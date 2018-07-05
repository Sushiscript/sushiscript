#include "./util.h"

using namespace sushi::lexer::test;
using sushi::lexer::Error;

TEST(ErrorHandling, TestUnknownChar) {
    NoIndentStrIsToks("\tident", TD(kUnknownChar, '\t'), TD(kIdent, "ident"));
    StringIsTokens(
        "  \t  ident", TD(kIndent, 2), TD(kUnknownChar, '\t'),
        TD(kIdent, "ident"));
    NoIndentStrIsToks(
        "ide\tnt", TD(kIdent, "ide"), TD(kUnknownChar, '\t'), TD(kIdent, "nt"));
}

TEST(ErrorHandling, TestUnclosedString) {
    NoIndentStrIsToks<true>(
        R"("unclosed string)", TL(kStringLit, 1, 1),
        TDL(kSegment, "unclosed string", 1, 2),
        TEL(kUnclosedStringQuote, 1, 17), TL(kInterDone, 1, 17));
    NoIndentStrIsToks<true>(
        "\"cannot close in next line\n\"", TL(kStringLit, 1, 1),
        TDL(kSegment, "cannot close in next line", 1, 2),
        TEL(kUnclosedStringQuote, 1, 27), TL(kInterDone, 1, 27),
        TL(kLineBreak, 1, 27), TDL(kIndent, 0, 2, 1), TL(kStringLit, 2, 1),
        TEL(kUnclosedStringQuote, 2, 2), TL(kInterDone, 2, 2));
    NoIndentStrIsToks<true>(
        "\"error\nrecovery", TL(kStringLit, 1, 1), TDL(kSegment, "error", 1, 2),
        TEL(kUnclosedStringQuote, 1, 7), TL(kInterDone, 1, 7),
        TL(kLineBreak, 1, 7), TDL(kIndent, 0, 2, 1),
        TDL(kIdent, "recovery", 2, 1));
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
    NoIndentStrIsToks<true>(
        "..hahaha", TL(kPathLit, 1, 1), TEL(kPathExpectSlash, 1, 3),
        TL(kInterDone, 1, 9));
    NoIndentStrIsToks<true>(
        "~hahaha", TL(kPathLit, 1, 1), TEL(kPathExpectSlash, 1, 2),
        TL(kInterDone, 1, 8));
    NoIndentStrIsToks<true>(
        "~~hahaha", TL(kPathLit, 1, 1), TEL(kPathExpectSlash, 1, 2),
        TL(kInterDone, 1, 9));
    NoIndentStrIsToks<true>(
        ".hahaha", TL(kPathLit, 1, 1), TEL(kPathExpectSlash, 1, 2),
        TL(kInterDone, 1, 8));
    NoIndentStrIsToks<true>(
        ".${", TL(kPathLit, 1, 1), TEL(kPathExpectSlash, 1, 2),
        TL(kInterStart, 1, 2));
}
