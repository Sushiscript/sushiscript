#ifndef SUSHI_LEXER_TOKEN_LOCATION_H_
#define SUSHI_LEXER_TOKEN_LOCATION_H_

#include <string>
#include <tuple>

namespace sushi {

// location of the first character of a token
struct TokenLocation {
    std::string src_path;
    int line;
    int column;

    TokenLocation() = delete;

    bool operator<(const TokenLocation &rhs) const {
        // TODO: take src_path into account
        return std::tie(line, column) < std::tie(rhs.line, rhs.column);
    }
    bool operator==(const TokenLocation &rhs) const {
        return std::tie(line, column, src_path) ==
               std::tie(line, column, src_path);
    }
    bool operator!=(const TokenLocation &rhs) const {
        return not(*this == rhs);
    }
    bool operator<=(const TokenLocation &rhs) const {
        return *this < rhs or *this == rhs;
    }
    bool operator>(const TokenLocation &rhs) const {
        return not(*this == rhs) and not(*this < rhs);
    }
    bool operator>=(const TokenLocation &rhs) const {
        return not(*this < rhs);
    }

    void NextColumn() {
        ++column;
    }

    void NewLine() {
        ++line;
        column = 0;
    }
};

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_LOCATION_H_
