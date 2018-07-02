#include "./util.h"

using namespace sushi::lexer::test;

TEST(InterpolationTest, TestStringLiteral) {
    NoIndentStrIsToks(R"("")", TK(kStringLit), TK(kInterDone));
    NoIndentStrIsToks(
        R"("haha")", TK(kStringLit), TD(kSegment, "haha"), TK(kInterDone));
    NoIndentStrIsToks(
        R"("ha\nha")", TK(kStringLit), TD(kSegment, "ha\nha"), TK(kInterDone));
    NoIndentStrIsToks(
        R"("ha\tha")", TK(kStringLit), TD(kSegment, "ha\tha"), TK(kInterDone));
    NoIndentStrIsToks(
        R"("say\"hello\"")", TK(kStringLit), TD(kSegment, "say\"hello\""),
        TK(kInterDone));
    NoIndentStrIsToks(
        R"("\${i}")", TK(kStringLit), TD(kSegment, "${i}"), TK(kInterDone));
    NoIndentStrIsToks(
        R"("$ {i}")", TK(kStringLit), TD(kSegment, "$ {i}"), TK(kInterDone));
}

TEST(InterpolationTest, TestString) {
    NoIndentStrIsToks<true>(
        R"("${ident})", TL(kStringLit, 1, 1), TL(kInterStart, 1, 2),
        TDL(kIdent, "ident", 1, 4), TL(kRBrace, 1, 9));
    NoIndentStrIsToks<true>(
        R"("${  ident, 123})", TL(kStringLit, 1, 1), TL(kInterStart, 1, 2),
        TDL(kIdent, "ident", 1, 6), TL(kComma, 1, 11), TDL(kIntLit, 123, 1, 13),
        TL(kRBrace, 1, 16));
    NoIndentStrIsToks<true>(
        R"("some${ident)", TL(kStringLit, 1, 1), TDL(kSegment, "some", 1, 2),
        TL(kInterStart, 1, 6), TDL(kIdent, "ident", 1, 8));
    NoIndentStrIsToks<true>(
        R"("${"str")", TL(kStringLit, 1, 1), TL(kInterStart, 1, 2),
        TL(kStringLit, 1, 4), TDL(kSegment, "str", 1, 5), TL(kInterDone, 1, 8));
}

TEST(InterpolationTest, TestRawToken) {
    RawStrIsTokens("1", TK(kRawString), TD(kSegment, "1"), TK(kInterDone));
    RawStrIsTokens("a", TK(kRawString), TD(kSegment, "a"), TK(kInterDone));
    RawStrIsTokens("?", TK(kRawString), TD(kSegment, "?"), TK(kInterDone));
    RawStrIsTokens("\\n", TK(kRawString), TD(kSegment, "n"), TK(kInterDone));
    RawStrIsTokens("\\;", TK(kRawString), TD(kSegment, ";"), TK(kInterDone));
    RawStrIsTokens(
        "\\;\\?", TK(kRawString), TD(kSegment, ";?"), TK(kInterDone));
    RawStrIsTokens("\\ ", TK(kRawString), TD(kSegment, " "), TK(kInterDone));
    RawStrIsTokens(
        "h\\a\\ haha ", TK(kRawString), TD(kSegment, "ha haha"),
        TK(kInterDone));
    RawStrIsTokens(
        "h\\a\\ haha ", TK(kRawString), TD(kSegment, "ha haha"),
        TK(kInterDone));
    RawStrIsTokens(
        R"(i\${i)", TK(kRawString), TD(kSegment, "i${i"), TK(kInterDone));
    RawStrIsTokens(
        R"(i\${i})", TK(kRawString), TD(kSegment, "i${i"), TK(kInterDone), TK(kRBrace));
    RawStrIsTokens(
        R"(i{i)", TK(kRawString), TD(kSegment, "i{i"), TK(kInterDone));
    RawStrIsTokens(
        R"(i{i})", TK(kRawString), TD(kSegment, "i{i"), TK(kInterDone), TK(kRBrace));
    RawStrIsTokens(
        R"(h$h\{i\})", TK(kRawString), TD(kSegment, "h$h{i}"), TK(kInterDone));
}

TEST(InterpolationTest, TestRaw) {
    RawStrIsTokens<true>(
        "raw..${ident}", TL(kRawString, 1, 1), TDL(kSegment, "raw..", 1, 1),
        TL(kInterStart, 1, 6), TDL(kIdent, "ident", 1, 8), TL(kRBrace, 1, 13));
    RawStrIsTokens<true>(
        "--${ ident, 123}", TL(kRawString, 1, 1), TDL(kSegment, "--", 1, 1),
        TL(kInterStart, 1, 3), TDL(kIdent, "ident", 1, 6), TL(kComma, 1, 11),
        TDL(kIntLit, 123, 1, 13), TL(kRBrace, 1, 16));
    RawStrIsTokens<true>(
        "${ident}", TL(kRawString, 1, 1), TL(kInterStart, 1, 1),
        TDL(kIdent, "ident", 1, 3), TL(kRBrace, 1, 8));
}

TEST(InterpolationTest, TestPathLiteral) {
    NoIndentStrIsToks(".", TK(kPathLit), TD(kSegment, "."), TK(kInterDone));
    NoIndentStrIsToks("/", TK(kPathLit), TD(kSegment, "/"), TK(kInterDone));
    NoIndentStrIsToks("..", TK(kPathLit), TD(kSegment, ".."), TK(kInterDone));
    NoIndentStrIsToks(
        "....", TK(kPathLit), TD(kSegment, "...."), TK(kInterDone));
    NoIndentStrIsToks("~", TK(kPathLit), TD(kSegment, "~"), TK(kInterDone));
    NoIndentStrIsToks("~/", TK(kPathLit), TD(kSegment, "~/"), TK(kInterDone));
    NoIndentStrIsToks("../", TK(kPathLit), TD(kSegment, "../"), TK(kInterDone));
    NoIndentStrIsToks(
        "../hello/world", TK(kPathLit), TD(kSegment, "../hello/world"),
        TK(kInterDone));
    NoIndentStrIsToks(
        "~/hello/world", TK(kPathLit), TD(kSegment, "~/hello/world"),
        TK(kInterDone));
    NoIndentStrIsToks(
        "~/hello\\ world", TK(kPathLit), TD(kSegment, "~/hello world"),
        TK(kInterDone));
    NoIndentStrIsToks(
        "../    ", TK(kPathLit), TD(kSegment, "../"), TK(kInterDone));
}

TEST(InterpolationTest, TestPath) {
    NoIndentStrIsToks<true>(
        "../${ident}", TL(kPathLit, 1, 1), TDL(kSegment, "../", 1, 1),
        TL(kInterStart, 1, 4), TDL(kIdent, "ident", 1, 6), TL(kRBrace, 1, 11));
    NoIndentStrIsToks<true>(
        "../\\ ${ident}", TL(kPathLit, 1, 1), TDL(kSegment, "../ ", 1, 1),
        TL(kInterStart, 1, 6), TDL(kIdent, "ident", 1, 8), TL(kRBrace, 1, 13));
    NoIndentStrIsToks<true>(
        "~/${ ident, 123}", TL(kPathLit, 1, 1), TDL(kSegment, "~/", 1, 1),
        TL(kInterStart, 1, 3), TDL(kIdent, "ident", 1, 6), TL(kComma, 1, 11),
        TDL(kIntLit, 123, 1, 13), TL(kRBrace, 1, 16));
}
