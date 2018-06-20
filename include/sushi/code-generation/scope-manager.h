#ifndef SUSHI_SCOPE_MANAGER_H_
#define SUSHI_SCOPE_MANAGER_H_
#include <string>
#include <map>
#include "sushi/scope.h"
#include "boost/format.hpp"

namespace sushi {
namespace code_generation {

class ScopeManager {
    std::map<const std::string &, int> origin_name_to_int;
    std::map<std::pair<const scope::Scope *, const std::string &>, std::string> new_names_map;
  public:
    ScopeManager() {
        origin_name_to_int["_sushi_t_"] = -1;
    }

    std::string GetNewTemp() {
        std::string str = "_sushi_t_";
        int new_int = origin_name_to_int[str];
        ++origin_name_to_int[str];
        if (new_int == -1) {
            return str;
        } else {
            auto new_name = str + std::to_string(new_int) + '_';
            return new_name;
        }
    }

    std::string GetNewName(const std::string & identifier, const scope::Scope * scope) {
        if (origin_name_to_int.find(identifier) != origin_name_to_int.end()
        || origin_name_to_int[identifier] == -1) {
            new_names_map[std::make_pair(scope, identifier)] = identifier;
            return identifier;
        } else {
            int new_int = origin_name_to_int[identifier];
            auto new_name = identifier + "_scope_" + std::to_string(new_int);
            new_names_map[std::make_pair(scope, identifier)] = new_name;
            ++origin_name_to_int[identifier];
            return new_name;
        }
    }

    void UnsetTemp(const std::string & new_name) {
        --origin_name_to_int[new_name];
    }

    void UnsetName(const std::string & new_name) {
        --origin_name_to_int[new_name];
        origin_name_to_int.erase(new_name);
    }

    std::string FindNewName(const std::string & identifier, const scope::Scope * scope) {
        auto find_res = new_names_map.find(std::make_pair(scope, identifier));
        if (find_res != new_names_map.end()) {
            return find_res->second;
        } else {
            throw "Cannot find the identifier";
        }
    }
};

} // namespace code_generation
} // namespace sushi

#endif
