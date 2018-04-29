#ifndef SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H
#define SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H

#include "boost/optional.hpp"
#include <string>
#include <unordered_map>

namespace sushi {
namespace lexer {

// special character make use of unused ascii
enum SpecialChar : char {
    kInterDollar = 1,
    kInterLBrace = 2,
    kInterRBrace = 3
};

namespace detail {

#define TRADITION_ESCAPE                                                       \
    {'a', '\a'}, {'b', '\b'}, {'f', '\f'}, {'n', '\n'}, {'r', '\r'},           \
        {'t', '\t'}, {                                                         \
        'v', '\v'                                                              \
    }

#define INTERPOLATE_ESCAPE                                                     \
    {'{', SpecialChar::kInterLBrace}, {'}', SpecialChar::kInterRBrace}, {      \
        '$', SpecialChar::kInterDollar                                         \
    }

struct CharacterConfig {
    bool Restrict(char c) const {
        return RestrictedList().find(c) != std::string::npos;
    }
    bool Prohibit(char c) const {
        return not isprint(c);
    }
    virtual boost::optional<char> Escape(char c) const {
        if (Prohibit(c)) return boost::none;
        auto it = EscapeMap().find(c);
        return it == end(EscapeMap()) ? c : it->second;
    }

  private:
    virtual const std::string &RestrictedList() const = 0;
    virtual const std::unordered_map<char, char> &EscapeMap() const = 0;
};

struct StringConfig : CharacterConfig {
  private:
    const std::string &RestrictedList() const override {
        static std::string l = "\"";
        return l;
    }
    const std::unordered_map<char, char> &EscapeMap() const override {
        static std::unordered_map<char, char> m = {TRADITION_ESCAPE,
                                                   INTERPOLATE_ESCAPE};
        return m;
    };
};

struct CharConfig : CharacterConfig {
  private:
    const std::string &RestrictedList() const override {
        static std::string l = "'";
        return l;
    }
    const std::unordered_map<char, char> &EscapeMap() const override {
        static std::unordered_map<char, char> m = {TRADITION_ESCAPE};
        return m;
    }
};

struct RawConfig : CharacterConfig {
  private:
    const std::string &RestrictedList() const override {
        static std::string l = "\"#'; ";
        return l;
    }
    const std::unordered_map<char, char> &EscapeMap() const override {
        static std::unordered_map<char, char> m = {INTERPOLATE_ESCAPE};
        return m;
    }
};

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H
