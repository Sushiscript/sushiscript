#include "sushi/parser/detail/token-util.h"
#include <algorithm>
#include <stdexcept>

namespace sushi {
namespace parser {
namespace detail {

using lexer::Token;
using type::BuiltInAtom;

#define TT(t) Token::Type::t

#define TOKEN_IN(t, ...)                                                       \
    Token::Type ts[] = {__VA_ARGS__};                                          \
    return std::find(std::begin(ts), std::end(ts), t) != std::end(ts);

bool IsBinaryOp(Token::Type t) {
    TOKEN_IN(
        t, TT(kPlus), TT(kMinus), TT(kStar), TT(kDivide), TT(kPercent),
        TT(kLAngle), TT(kRAngle), TT(kGreaterEq), TT(kLessEq), TT(kDoubleEq),
        TT(kNotEqual), TT(kOr), TT(kAnd));
}

int BinaryOpPrec(Token::Type t) {
    switch (t) {
    case TT(kStar):
    case TT(kDivide):
    case TT(kPercent): return 4;
    case TT(kPlus):
    case TT(kMinus): return 3;
    case TT(kLAngle):
    case TT(kRAngle):
    case TT(kLessEq):
    case TT(kGreaterEq): return 2;
    case TT(kDoubleEq):
    case TT(kNotEqual): return 1;
    case TT(kOr):
    case TT(kAnd): return 0;
    default:
        throw std::invalid_argument(
            "BinaryOpPrec: invalid binary operator: " + Token::TypeToString(t));
    }
}

ast::BinaryExpr::Operator BinOpTokenToOperator(Token::Type t) {
    using Op = ast::BinaryExpr::Operator;
    switch (t) {
    case TT(kStar): return Op::kMult;
    case TT(kDivide): return Op::kDiv;
    case TT(kPercent): return Op::kMod;
    case TT(kPlus): return Op::kAdd;
    case TT(kMinus): return Op::kMinus;
    case TT(kLAngle): return Op::kLess;
    case TT(kRAngle): return Op::kGreat;
    case TT(kLessEq): return Op::kLessEq;
    case TT(kGreaterEq): return Op::kGreatEq;
    case TT(kDoubleEq): return Op::kEqual;
    case TT(kNotEqual): return Op::kNotEq;
    case TT(kOr): return Op::kOr;
    case TT(kAnd): return Op::kAnd;
    default:
        throw std::invalid_argument(
            "BinOpTokenToOperator: invalid binary operator: " +
            Token::TypeToString(t));
    }
}

bool IsUnaryOp(Token::Type t) {
    TOKEN_IN(t, TT(kNot), TT(kPlus), TT(kMinus));
}

ast::UnaryExpr::Operator UnaryOpTokenToOperator(Token::Type t) {
    using Op = ast::UnaryExpr::Operator;
    switch (t) {
    case TT(kPlus): return Op::kPos;
    case TT(kMinus): return Op::kNeg;
    case TT(kNot): return Op::kNot;
    default:
        throw std::invalid_argument(
            "UnaryOpTokenToOperator: invalid unary operator: " +
            Token::TypeToString(t));
    }
}

bool IsSimpleType(Token::Type t) {
    TOKEN_IN(
        t, TT(kInt), TT(kBool), TT(kUnit), TT(kChar), TT(kString), TT(kPath),
        TT(kRelPath), TT(kExitCode), TT(kFd));
}

bool IsType(Token::Type t) {
    TOKEN_IN(
        t, TT(kInt), TT(kBool), TT(kUnit), TT(kChar), TT(kString), TT(kPath),
        TT(kRelPath), TT(kArray), TT(kMap), TT(kExitCode), TT(kFd),
        TT(kFunction));
}

bool IsTypeLookahead(Token::Type t) {
    return IsType(t) or t == TT(kLParen);
}

BuiltInAtom::Type TypeTokenToType(Token::Type t) {
    using TP = type::BuiltInAtom::Type;
    switch (t) {
    case TT(kInt): return TP::kInt;
    case TT(kBool): return TP::kBool;
    case TT(kUnit): return TP::kUnit;
    case TT(kFd): return TP::kFd;
    case TT(kExitCode): return TP::kExitCode;
    case TT(kPath): return TP::kPath;
    case TT(kRelPath): return TP::kRelPath;
    case TT(kChar): return TP::kChar;
    case TT(kString): return TP::kString;
    default:
        throw std::invalid_argument(
            "TypeTokenToType: invalid simple type: " + Token::TypeToString(t));
    }
}

bool IsLiteral(Token::Type t) {
    TOKEN_IN(
        t, TT(kCharLit), TT(kStringLit), TT(kPathLit), TT(kIntLit), TT(kTrue),
        TT(kFalse), TT(kUnit), TT(kStdin), TT(kStdout), TT(kStderr));
}

bool IsBoolLiteral(Token::Type t) {
    TOKEN_IN(t, TT(kTrue), TT(kFalse));
}

bool BoolLitToBool(lexer::Token::Type t) {
    if (t == Token::Type::kTrue)
        return true;
    else if (t == Token::Type::kFalse)
        return false;
    throw std::invalid_argument(
        "BoolLitToBool: invalid bool: " + Token::TypeToString(t));
}

bool IsFdLiteral(lexer::Token::Type t) {
    TOKEN_IN(t, TT(kStdin), TT(kStdout), TT(kStderr));
}

ast::FdLit::Value FdLiteralToFd(lexer::Token::Type t) {
    using V = ast::FdLit::Value;
    switch (t) {
    case Token::Type::kStdin: return V::kStdin;
    case Token::Type::kStdout: return V::kStdout;
    case Token::Type::kStderr: return V::kStderr;
    default:
        throw std::invalid_argument(
            "BoolLitToBool: invalid fd: " + Token::TypeToString(t));
    }
}

bool IsError(Token::Type t) {
    TOKEN_IN(t, TT(kUnknownChar), TT(kInvalidChar), TT(kErrorCode));
}

bool IsSpace(Token::Type t) {
    TOKEN_IN(t, TT(kIndent), TT(kLineBreak));
}

bool IsStatementEnd(Token::Type t) {
    TOKEN_IN(t, TT(kSemicolon), TT(kLineBreak));
}
bool IsInterpolatable(Token::Type t) {
    TOKEN_IN(t, TT(kPathLit), TT(kRawString), TT(kStringLit));
}

bool IsAtomExprLookahead(Token::Type t) {
    return IsLiteral(t) or t == TT(kLParen) or t == TT(kLBrace) or
           t == TT(kIdent);
}

bool IsRawExit(lexer::Token::Type t) {
    TOKEN_IN(t, TT(kSemicolon), TT(kLineBreak), TT(kPipe));
}

bool IsExpressionLookahead(lexer::Token::Type t) {
    return IsUnaryOp(t) or IsAtomExprLookahead(t);
}

#undef TT

} // namespace detail
} // namespace parser
} // namespace sushi