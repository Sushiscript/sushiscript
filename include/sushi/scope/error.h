#ifndef SUSHI_SCOPE_ERROR_H_
#define SUSHI_SCOPE_ERROR_H_

#include "sushi/lexer/token.h"
#include <iostream>

namespace sushi {
namespace scope {

struct Error {
    enum class Type {
        kNoIdentyInfoError
    };
    static std::string ToString(Type t) {
        switch (t) {
        case Type::kNoIdentyInfoError: return "NoIdentyInfoError";
        }
    }
    std::string ToString() const {
        // return '[' + Error::ToString(type) + ',' + position.ToString() + ']';
        return '[' + Error::ToString(type) + ',' + ']';
    }
    // bool operator==(const Error& rhs) const {
    //     return type == rhs.type and position == rhs.position;
    // }
    Type type;
    // lexer::Token position;
};

inline std::ostream& operator<<(std::ostream& os, const Error& err) {
    os << err.ToString();
    return os;
}

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_ERROR_H_
