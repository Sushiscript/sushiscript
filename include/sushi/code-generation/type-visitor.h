#ifndef SUSHI_CODE_GEN_TYPE_VISITOR_H_
#define SUSHI_CODE_GEN_TYPE_VISITOR_H_

#include "sushi/ast.h"

namespace sushi {
namespace code_generation {

struct CodeGenTypeExprVisitor : public ast::TypeExprVisitor::Const {
    std::string type_expr_str;

    SUSHI_VISITING(ast::TypeLit, type_lit) {}
    SUSHI_VISITING(ast::ArrayType, array_type) {
        type_expr_str = "a";
    }
    SUSHI_VISITING(ast::MapType, map_type) {
        type_expr_str = "A";
    }
    SUSHI_VISITING(ast::FunctionType, function_type) {}
};



struct CodeGenTypeVisitor : public type::TypeVisitor::Const {
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

} // namespace code_generation
} // namespace sushi

#endif
