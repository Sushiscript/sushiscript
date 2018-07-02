#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include "./scope-manager.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "sushi/ast.h"

namespace sushi {
namespace code_generation {

constexpr char kIndentString[] = "    ";

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program &program, const scope::Environment &environment,
        std::shared_ptr<ScopeManager> scope_manager = nullptr);

    static std::string AddIndentToEachLine(const std::string &str) {
        std::vector<std::string> res;
        boost::algorithm::split(res, str, boost::algorithm::is_any_of("\n"));
        std::string ret;
        for (auto &s : res) {
            if (s.empty())
                ret += '\n';
            else
                ret += kIndentString + s + '\n';
        }
        return ret;
    }
};

} // namespace code_generation
} // namespace sushi

#endif
