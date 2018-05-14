#ifndef SUSHI_PARSER_DETAIL_TOKEN_UTIL_H_
#define SUSHI_PARSER_DETAIL_TOKEN_UTIL_H_

#include "sushi/ast/expression/binary-expr.h"
#include "sushi/ast/expression/literal.h"
#include "sushi/ast/expression/unary-expr.h"
#include "sushi/lexer/token.h"
#include "sushi/type-system/type.h"

namespace sushi {
namespace parser {
namespace detail {

bool IsBinaryOp(lexer::Token::Type);

ast::BinaryExpr::Operator BinOpTokenToOperator(lexer::Token::Type t);

int BinaryOpPrec(lexer::Token::Type);

bool IsUnaryOp(lexer::Token::Type);

ast::UnaryExpr::Operator UnaryOpTokenToOperator(lexer::Token::Type t);

bool IsSimpleType(lexer::Token::Type t);

bool IsType(lexer::Token::Type);

bool IsTypeLookahead(lexer::Token::Type t);

type::BuiltInAtom::Type TypeTokenToType(lexer::Token::Type);

bool IsLiteral(lexer::Token::Type);

bool IsBoolLiteral(lexer::Token::Type);

bool BoolLitToBool(lexer::Token::Type);

bool IsFdLiteral(lexer::Token::Type);

ast::FdLit::Value FdLiteralToFd(lexer::Token::Type t);

bool IsError(lexer::Token::Type t);

bool IsSpace(lexer::Token::Type);

bool IsStatementEnd(lexer::Token::Type);

bool IsInterpolatable(lexer::Token::Type);

bool IsAtomExprLookahead(lexer::Token::Type t);

bool IsRawExit(lexer::Token::Type t);

bool IsExpressionLookahead(lexer::Token::Type t);

} // namespace detail
} // namespace parser
} // namespace sushi

#endif // SUSHI_PARSER_TOKEN_UITL_H_