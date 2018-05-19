#ifndef SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H
#define SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H

#include "boost/optional.hpp"
#include <memory>
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

struct CharacterConfig {
    bool Restrict(char c) const {
        return RestrictedList().find(c) != std::string::npos;
    }
    bool Prohibit(char c) const {
        return not isprint(c);
    }
    boost::optional<char> Unescape(char c) const {
        if (Restrict(c) or Prohibit(c)) return boost::none;
        auto it = UnescapeSpecial().find(c);
        return it == end(UnescapeSpecial()) ? c : it->second;
    }
    boost::optional<char> Escape(char c) const {
        if (Prohibit(c)) return boost::none;
        auto it = EscapeMap().find(c);
        return it == end(EscapeMap()) ? c : it->second;
    }

    virtual const std::string &RestrictedList() const {
        static std::string s = "";
        return s;
    };
    virtual const std::unordered_map<char, char> &EscapeMap() const {
        static std::unordered_map<char, char> m;
        return m;
    }
    virtual const std::unordered_map<char, char> &UnescapeSpecial() const {
        static std::unordered_map<char, char> m;
        return m;
    }

    virtual ~CharacterConfig() = default;
};

struct StringConfig : CharacterConfig {
    const std::string &RestrictedList() const override {
        static std::string l = "\"";
        return l;
    }
    const std::unordered_map<char, char> &EscapeMap() const override {
        static std::unordered_map<char, char> m = {TRADITION_ESCAPE};
        return m;
    };
};

struct CharConfig : CharacterConfig {
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
    const std::string &RestrictedList() const override {
        static std::string l = "\"#'; |,)";
        return l;
    }
};

// decorator
struct CustomConfig : CharacterConfig {
    CustomConfig(
        std::unique_ptr<CharacterConfig> base, const std::string &restr,
        std::unordered_map<char, char> escape = {},
        std::unordered_map<char, char> special = {})
        : restr_list_(restr), escape_(std::move(escape)),
          special_(std::move(special)) {
        if (base == nullptr) return;
        restr_list_ += base->RestrictedList();
        SafeMerge(escape_, base->EscapeMap());
        SafeMerge(special_, base->UnescapeSpecial());
    }

    static void SafeMerge(
        std::unordered_map<char, char> &me,
        const std::unordered_map<char, char> &that) {
        for (auto p : that)
            if (not me.count(p.first)) me.insert(p);
    }

    const std::string &RestrictedList() const override {
        return restr_list_;
    };
    const std::unordered_map<char, char> &EscapeMap() const override {
        return escape_;
    }
    const std::unordered_map<char, char> &UnescapeSpecial() const override {
        return special_;
    }

  private:
    std::string restr_list_;
    std::unordered_map<char, char> escape_;
    std::unordered_map<char, char> special_;
};

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_DETAIL_CHARACTER_CONFIG_H
