#include "./util.h"

using namespace sushi::lexer::test;

TEST(SimpleSequence, TestLineBreak) {
    NoIndentStrIsToks("1\n", TD(kIntLit, 1), TK(kLineBreak));
    NoIndentStrIsToks("1 \n", TD(kIntLit, 1), TK(kLineBreak));
    NoIndentStrIsToks(
        "ident, \n", TD(kIdent, "ident"), TK(kComma), TK(kLineBreak));
    RawStrIsTokens("\n", TK(kLineBreak));
    RawStrIsTokens(
        "some/raw/token and \n", TK(kRawString), TD(kSegment, "some/raw/token"),
        TK(kInterDone), TK(kRawString), TD(kSegment, "and"), TK(kInterDone),
        TK(kLineBreak));
}

TEST(SimpleSequence, TestComment) {
    NoIndentStrIsToks("ident # hello there", TD(kIdent, "ident"));
    NoIndentStrIsToks("# hello there\nident", TD(kIdent, "ident"));
}

TEST(SimpleSequence, TestWhitespaces) {
    StringIsTokens(
        "    4 Indents", TD(kIndent, 4), TD(kIntLit, 4), TD(kIdent, "Indents"));
    StringIsTokens(
        "    4 Indents\n  2 Indents", TD(kIndent, 4), TD(kIntLit, 4),
        TD(kIdent, "Indents"), TK(kLineBreak), TD(kIndent, 2), TD(kIntLit, 2),
        TD(kIdent, "Indents"));
    StringIsTokens(
        "     \n  2 Indents", TD(kIndent, 2), TD(kIntLit, 2),
        TD(kIdent, "Indents"));
    StringIsTokens(
        "  \n \n  2 Indents", TD(kIndent, 2), TD(kIntLit, 2),
        TD(kIdent, "Indents"));
    StringIsTokens(
        "  # comment\n  2 Indents", TD(kIndent, 2), TD(kIntLit, 2),
        TD(kIdent, "Indents"));
    StringIsTokens(
        "# comment\n  2 Indents", TD(kIndent, 2), TD(kIntLit, 2),
        TD(kIdent, "Indents"));
    StringIsTokens(
        "   \\\n  line join", TD(kIndent, 2), TD(kIdent, "line"),
        TD(kIdent, "join"));
}

TEST(SimpleSequence, TestLocation) {
    NoIndentStrIsToks<true>("ident", TDL(kIdent, "ident", 1, 1));
    ExactStrIsToks(
        "\nident", TDL(kIndent, 0, 2, 1), TDL(kIdent, "ident", 2, 1));
    ExactStrIsToks(
        "  ident", TDL(kIndent, 2, 1, 1), TDL(kIdent, "ident", 1, 3));
    ExactStrIsToks(
        "\n  ident", TDL(kIndent, 2, 2, 1), TDL(kIdent, "ident", 2, 3));
    ExactStrIsToks(
        "\n  ident", TDL(kIndent, 2, 2, 1), TDL(kIdent, "ident", 2, 3));
    ExactStrIsToks(
        "\n  \n  \n  ident", TDL(kIndent, 2, 4, 1), TDL(kIdent, "ident", 4, 3));
    NoIndentStrIsToks<true>(
        "ident 123 true", TDL(kIdent, "ident", 1, 1), TDL(kIntLit, 123, 1, 7),
        TL(kTrue, 1, 11));
    NoIndentStrIsToks<true>(
        "== != < >", TL(kDoubleEq, 1, 1), TL(kNotEqual, 1, 4),
        TL(kLAngle, 1, 7), TL(kRAngle, 1, 9));
    RawStrIsTokens(
        "$IDENT  ${", TL(kRawString, 1, 1), TDL(kSegment, "$IDENT", 1, 1),
        TL(kInterDone, 1, 7), TL(kRawString, 1, 9), TL(kInterStart, 1, 9));
}

TEST(SimpleSequence, TestConnected) {
    NoIndentStrIsToks(
        "1<2==true", TD(kIntLit, 1), TK(kLAngle), TD(kIntLit, 2), TK(kDoubleEq),
        TK(kTrue));
    NoIndentStrIsToks(
        "===>====", TK(kDoubleEq), TK(kSingleEq), TK(kGreaterEq), TK(kDoubleEq),
        TK(kSingleEq));
    NoIndentStrIsToks("line\\\njoin", TD(kIdent, "line"), TD(kIdent, "join"));
    NoIndentStrIsToks(
        R"(./path/and"string")", TK(kPathLit), TD(kSegment, "./path/and"),
        TK(kInterDone), TK(kStringLit), TD(kSegment, "string"), TK(kInterDone));
    NoIndentStrIsToks(
        "./path/and'c'", TK(kPathLit), TD(kSegment, "./path/and"),
        TK(kInterDone), TD(kCharLit, 'c'));
    NoIndentStrIsToks(
        "./path/and#comment", TK(kPathLit), TD(kSegment, "./path/and"),
        TK(kInterDone));
}
