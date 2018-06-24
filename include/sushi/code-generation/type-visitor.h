#ifndef SUSHI_CODE_GEN_TYPE_VISITOR_H_
#define SUSHI_CODE_GEN_TYPE_VISITOR_H_

#include "sushi/ast.h"

namespace sushi {
namespace code_generation {

struct TypeVisitor : public type::TypeVisitor::Const {
    enum class SimplifiedType {
        kInt,
        kBool,
        kUnit,
        kFd,
        kExitCode,
        kPath,
        kRelPath,
        kString,
        kChar,
        kArray,
        kMap,
        kFunc
    };

    inline static SimplifiedType BuiltInAtomTypeToSimplifiedType(
        type::BuiltInAtom::Type built_in_type) {
        using T = type::BuiltInAtom::Type;
        using ST = SimplifiedType;
        switch (built_in_type) {
            case T::kInt: return ST::kInt;
            case T::kBool: return ST::kBool;
            case T::kUnit: return ST::kUnit;
            case T::kFd: return ST::kFd;
            case T::kExitCode: return ST::kExitCode;
            case T::kPath: return ST::kPath;
            case T::kRelPath: return ST::kRelPath;
            case T::kString: return ST::kString;
            case T::kChar: return ST::kChar;
        }
    }

    SimplifiedType type;

    SUSHI_VISITING(type::BuiltInAtom, built_in_atom) {
        type = BuiltInAtomTypeToSimplifiedType(built_in_atom.type);
    }
    SUSHI_VISITING(type::Array, array) {
        type = SimplifiedType::kArray;
    }
    SUSHI_VISITING(type::Map, map) {
        type = SimplifiedType::kMap;
    }
    SUSHI_VISITING(type::Function, function) {
        type = SimplifiedType::kFunc;
    }
};

struct TypeExprVisitor : public ast::TypeExprVisitor::Const {
    using ST = TypeVisitor::SimplifiedType;
    using BT = type::BuiltInAtom::Type;

    ST type;

    SUSHI_VISITING(ast::TypeLit, type_lit) {
        switch (type_lit.type) {
        case BT::kInt: type = ST::kInt; break;
        case BT::kBool: type = ST::kBool; break;
        case BT::kUnit: type = ST::kUnit; break;
        case BT::kFd: type = ST::kFd; break;
        case BT::kExitCode: type = ST::kExitCode; break;
        case BT::kPath: type = ST::kPath; break;
        case BT::kRelPath: type = ST::kRelPath; break;
        case BT::kString: type = ST::kString; break;
        case BT::kChar: type = ST::kChar; break;
        }
    }
    SUSHI_VISITING(ast::ArrayType, array_type) {
        type = ST::kArray;
    }
    SUSHI_VISITING(ast::MapType, map_type) {
        type = ST::kMap;
    }
    SUSHI_VISITING(ast::FunctionType, function_type) {
        type = ST::kFunc;
    }
};

} // namespace code_generation
} // namespace sushi

#endif
