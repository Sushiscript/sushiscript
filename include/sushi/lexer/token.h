#ifndef SUSHI_LEXER_TOKEN_H_
#define SUSHI_LEXER_TOKEN_H_

#include "./token-location.h"
#include "boost/variant.hpp"
#include "./error.h"
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
        kAppend,
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
        kStringLitSeg, // string lit segment
        kPathLit,
        kPathLitSeg,   // path lit segment
        kIntLit,
        kRawString,
        kRawStringSeg, // raw string segment

        // special

        kIndent,
        kLineBreak,

        // error

        kOtherChar,
        kInvalidChar,
        kErrorCode
    };
    static std::string TypeToString(Type t) {
        switch (t) {
        case Type::kIdent: return "identifier";
        case Type::kOr: return "or";
        case Type::kNot: return "not";
        case Type::kAnd: return "and";
        case Type::kDefine: return "define";
        case Type::kReturn: return "return";
        case Type::kExport: return "export";
        case Type::kIf: return "if";
        case Type::kElse: return "else";
        case Type::kSwitch: return "switch";
        case Type::kCase: return "case";
        case Type::kDefault: return "default";
        case Type::kFor: return "for";
        case Type::kIn: return "in";
        case Type::kBreak: return "break";
        case Type::kContinue: return "continue";
        case Type::kRedirect: return "redirect";
        case Type::kFrom: return "from";
        case Type::kTo: return "to";
        case Type::kAppend: return "append";
        case Type::kHere: return "here";
        case Type::kInt: return "Int";
        case Type::kBool: return "Bool";
        case Type::kUnit: return "Unit";
        case Type::kChar: return "Char";
        case Type::kString: return "String";
        case Type::kPath: return "Path";
        case Type::kArray: return "Array";
        case Type::kMap: return "Map";
        case Type::kExitCode: return "ExitCode";
        case Type::kFd: return "Fd";
        case Type::kStdin: return "stdin";
        case Type::kStdout: return "stdout";
        case Type::kStderr: return "stderr";
        case Type::kUnitLit: return "unit";
        case Type::kTrue: return "true";
        case Type::kFalse: return "false";
        case Type::kPlus: return "+";
        case Type::kMinus: return "-";
        case Type::kStar: return "*";
        case Type::kDivide: return "//";
        case Type::kPercent: return "%";
        case Type::kLAngle: return "<";
        case Type::kRAngle: return ">";
        case Type::kGreaterEq: return ">=";
        case Type::kLessEq: return "<=";
        case Type::kDoubleEq: return "==";
        case Type::kNotEqual: return "!=";
        case Type::kComma: return ",";
        case Type::kSingleEq: return "=";
        case Type::kColon: return ":";
        case Type::kSemicolon: return ";";
        case Type::kExclamation: return "!";
        case Type::kDollar: return "$";
        case Type::kLParen: return "(";
        case Type::kRParen: return ")";
        case Type::kLBracket: return "[";
        case Type::kRBracket: return "]";
        case Type::kLBrace: return "{";
        case Type::kRBrace: return "}";
        case Type::kInterStart: return "${";
        case Type::kCharLit: return "CharLit";
        case Type::kStringLit: return "StrLit";
        case Type::kStringLitSeg: return "StrLitSeg";
        case Type::kPathLit: return "PathLit";
        case Type::kPathLitSeg: return "PathLitSeg";
        case Type::kIntLit: return "IntLit";
        case Type::kRawString: return "RawStr";
        case Type::kRawStringSeg: return "RawStrSeg";
        case Type::kIndent: return "Indent";
        case Type::kLineBreak: return "\\n";
        case Type::kOtherChar: return "Unknown";
        case Type::kInvalidChar: return "Invalid";
        case Type::kErrorCode: return "ErrCode";
        }
    }
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
            {"append", Type::kAppend},
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
            {"unit", Type::kUnitLit}};
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
            {'+', Type::kPlus},      {'-', Type::kMinus},
            {'*', Type::kStar},      {'%', Type::kPercent},
            {'<', Type::kLAngle},    {'>', Type::kRAngle},
            {',', Type::kComma},     {':', Type::kColon},
            {';', Type::kSemicolon}, {'!', Type::kExclamation},
            {'$', Type::kDollar},    {'(', Type::kLParen},
            {')', Type::kRParen},    {'[', Type::kLBracket},
            {']', Type::kRBracket},  {'{', Type::kLBrace},
            {'}', Type::kRBrace},    {'=', Type::kSingleEq}};
        return single_punct_map;
    }

    static Token FromIdent(const std::string &s, TokenLocation loc) {
        auto iter = IdentifierMap().find(s);
        if (iter == end(IdentifierMap())) {
            return Token{Type::kIdent, loc, s};
        }
        return Token{iter->second, loc, 0};
    }

    static bool WeakEqual(const Token &t1, const Token &t2) {
        return t1.type == t2.type and t1.content == t2.content;
    }

    static Token Error(TokenLocation l, lexer::Error e) {
        return {Type::kErrorCode, l, static_cast<int>(e)};
    }

    bool operator==(const Token &rhs) const {
        return WeakEqual(*this, rhs) and location == rhs.location;
    }

    using Data = boost::variant<int, std::string>;

    Token::Type type;
    TokenLocation location;
    Data content;
}; // namespace lexer

inline std::ostream &operator<<(std::ostream &os, lexer::Token tok) {
    os << "`" << lexer::Token::TypeToString(tok.type) << ", " << tok.content
       << ", (" << tok.location.line << ", " << tok.location.column << ")`";
    return os;
}

} // namespace lexer

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_H_
