#ifndef SUSHI_LEXER_TOKEN_H_
#define SUSHI_LEXER_TOKEN_H_

#include "./token-location.h"
#include "boost/variant.hpp"
#include <string>
#include <unordered_map>

namespace sushi {
namespace lexer {

struct Token {

    enum class Type {
        kIdent,

        // keywords

        kOr,
        kNot,
        kAnd,
        kDefine,
        kReturn,
        kExport,
        kIf,
        kElse,
        kSwitch,
        kCase,
        kDefault,
        kFor,
        kIn,
        kBreak,
        kContinue,
        kRedirect,
        kFrom,
        kTo,
        kHere,

        // built-in types

        kInt,
        kBool,
        kUnit,
        kChar,
        kString,
        kPath,
        kArray,
        kMap,
        kExitCode,
        kFd,

        // named literals

        kStdin,
        kStdout,
        kStderr,
        kUnitLit,
        kTrue,
        kFalse,

        // punctuations

        kPlus,        // +
        kMinus,       // -
        kStar,        // *
        kDivide,      // //
        kPercent,     // %
        kLAngle,      // <
        kRAngle,      // >
        kGreaterEq,   // >=
        kLessEq,      // <=
        kDoubleEq,    // ==
        kNotEqual,    // !=
        kComma,       // ,
        kSingleEq,    // =
        kColon,       // :
        kSemicolon,   // ;
        kExclamation, // !
        kDollar,      // $
        kLParen,      // (
        kRParen,      // )
        kLBracket,    // [
        kRBracket,    // ]
        kLBrace,      // {
        kRBrace,      // }
        kInterStart,  // ${

        // literals

        kCharLit,
        kStringLit,
        kPathLit,
        kIntLit,
        kRawString,

        // special

        kIndent,
        kLineBreak,

        // error

        kOtherChar,
        kInvalidChar,
        kErrorCode
    };
    static const std::unordered_map<std::string, Token::Type> &IdentifierMap() {
        static std::unordered_map<std::string, Token::Type> ident_map{
            {"or", Type::kOr},
            {"not", Type::kNot},
            {"and", Type::kAnd},
            {"define", Type::kDefine},
            {"return", Type::kReturn},
            {"export", Type::kExport},
            {"if", Type::kIf},
            {"else", Type::kElse},
            {"switch", Type::kSwitch},
            {"case", Type::kCase},
            {"default", Type::kDefault},
            {"for", Type::kFor},
            {"in", Type::kIn},
            {"break", Type::kBreak},
            {"continue", Type::kContinue},
            {"redirect", Type::kRedirect},
            {"from", Type::kFrom},
            {"to", Type::kTo},
            {"here", Type::kHere},
            {"Int", Type::kInt},
            {"Bool", Type::kBool},
            {"Unit", Type::kUnit},
            {"Char", Type::kChar},
            {"String", Type::kString},
            {"Path", Type::kPath},
            {"Array", Type::kArray},
            {"Map", Type::kMap},
            {"ExitCode", Type::kExitCode},
            {"FD", Type::kFd},
            {"stdin", Type::kStdin},
            {"stdout", Type::kStdout},
            {"stderr", Type::kStderr},
            {"true", Type::kTrue},
            {"false", Type::kFalse},
            {"unit", Type::kUnit}};
        return ident_map;
    }
    static const std::unordered_map<std::string, Token::Type> &
    DoublePunctuationMap() {
        static std::unordered_map<std::string, Token::Type> double_punct_map{
            {">=", Type::kGreaterEq},
            {"<=", Type::kLessEq},
            {"==", Type::kDoubleEq},
            {"!=", Type::kNotEqual},
            {"//", Type::kDivide}};
        return double_punct_map;
    }
    static const std::unordered_map<char, Token::Type> &SinglePunctuationMap() {
        static std::unordered_map<char, Token::Type> single_punct_map{
            {'+', Type::kPlus}, {'-', Type::kMinus},
            {'*', Type::kStar}, {'%', Type::kPercent},
            {'<', Type::kLAngle}, {'>', Type::kRAngle},
            {',', Type::kComma}, {':', Type::kColon},
            {';', Type::kSemicolon}, {'!', Type::kExclamation},
            {'$', Type::kDollar}, {'(', Type::kLParen},
            {')', Type::kRParen}, {'[', Type::kLBracket},
            {']', Type::kRBracket}, {'{', Type::kLBrace},
            {'}', Type::kRBrace}};
        return single_punct_map;
    }

    static Token FromIdent(const std::string &s, TokenLocation loc) {
        auto iter = IdentifierMap().find(s);
        if (iter == end(IdentifierMap())) {
            return Token{Type::kIdent, loc, s};
        }
        return Token{iter->second, loc, 0};
    }

    using Data = boost::variant<int, std::string>;

    Token::Type type;
    TokenLocation location;
    Data content;
};

} // namespace lexer

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_H_
