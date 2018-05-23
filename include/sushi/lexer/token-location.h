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

    static TokenLocation Eof(const std::string &path = "") {
        return {path, -1, -1};
    }

    bool operator<(const TokenLocation &rhs) const {
        // TODO: take src_path into account
        return std::tie(line, column) < std::tie(rhs.line, rhs.column);
    }
    bool operator==(const TokenLocation &rhs) const {
        return std::tie(line, column, src_path) ==
               std::tie(rhs.line, rhs.column, rhs.src_path);
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

    bool IsEof() const {
        return line == -1 and column == -1;
    }

    std::string ToString() const {
        if (IsEof()) return "EOF";
        std::string path;
        if (not src_path.empty()) path = "@" + path;
        return "(ln:" + std::to_string(line) +
               ",col:" + std::to_string(column) + path + ")";
    }

    void NextColumn() {
        ++column;
    }

    void NewLine() {
        ++line;
        column = 1;
    }
};

} // namespace sushi

#endif // SUSHI_LEXER_TOKEN_LOCATION_H_
