#include "./util.h"
#include "gtest/gtest.h"
#include <memory>
#include <sstream>
#include <string>

using namespace sushi::parser::detail;
using namespace sushi::lexer;

class LexerUtilTest : public ::testing::Test {
  protected:
    std::string TestSources() {
        return "  this \n is some test \n string";
    }
    virtual void SetUp() {
        is = std::make_unique<std::istringstream>(TestSources());
        lexer = std::make_unique<Lexer>(*is, TokenLocation{"", 1, 1});
    }
    std::unique_ptr<std::istream> is;
    std::unique_ptr<Lexer> lexer;
};

TEST_F(LexerUtilTest, TestSkipSpaceLookahead) {
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer), (TD(kIdent, "this")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer, 2), (TK(kLineBreak)));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer, 3), (TD(kIndent, 1)));
    lexer->Next();
    lexer->Next();
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer), (TD(kIdent, "is")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer, 2), (TD(kIdent, "some")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceLookahead(*lexer, 3), (TD(kIdent, "test")));
}

TEST_F(LexerUtilTest, TestSkipSpaceNext) {
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceNext(*lexer), (TD(kIdent, "this")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceNext(*lexer), (TD(kIdent, "is")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceNext(*lexer), (TD(kIdent, "some")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceNext(*lexer), (TD(kIdent, "test")));
    EXPECT_PRED2(
        Token::WeakEqual, *SkipSpaceNext(*lexer), (TD(kIdent, "string")));
}

TEST_F(LexerUtilTest, TestOptional) {
    auto a = Optional(*lexer, Token::Type::kIdent, false); // none
    auto b = Optional(*lexer, Token::Type::kIndent, false); // indent
    auto c = Optional(*lexer, Token::Type::kIndent, false); // none
    auto d = Optional(*lexer, Token::Type::kIdent, false); // ident
    EXPECT_EQ(a, boost::none);
    EXPECT_EQ(b->IntData(), 2);
    EXPECT_EQ(c, boost::none);
    EXPECT_EQ(d->StrData(), std::string("this"));
}