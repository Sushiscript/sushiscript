#include "./util.h"

using namespace sushi::lexer::test;

TEST(SimpleSequence, TestLineBreak) {
    NoIndentStrIsToks("1\n", TD(kIntLit, 1), TK(kLineBreak));
    NoIndentStrIsToks("1 \n", TD(kIntLit, 1), TK(kLineBreak));
    NoIndentStrIsToks(
        "ident, \n", TD(kIdent, "ident"), TK(kComma), TK(kLineBreak));
    RawStrIsTokens("\n", TK(kLineBreak));
    RawStrIsTokens(
        "some/raw/token and \n", TD(kRawString, "some/raw/token"),
        TD(kRawString, "and"), TK(kLineBreak));
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
}