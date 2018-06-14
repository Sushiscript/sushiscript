#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include "sushi/ast.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "./scope-manager.h"

namespace sushi {

constexpr char kIndentString[] = "    ";

class CodeGenerator {
  public:
    std::string GenCode(
        const ast::Program & program,
        const scope::Environment & environment,
        std::shared_ptr<ScopeManager> scope_manager = nullptr);

    static std::string AddIndentToEachLine(const std::string & str) {
        std::vector<std::string> res;
        boost::algorithm::split(res, str, boost::algorithm::is_any_of("\n"));
        std::string ret;
        for (auto & s : res) {
            ret += kIndentString + s + '\n';
        }
        return ret;
    }
};

}  // namespace sushi

#endif
