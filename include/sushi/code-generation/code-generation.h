#ifndef SUSHI_CODE_GENERATION_H_
#define SUSHI_CODE_GENERATION_H_

#include "./scope-manager.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "builtin-bash.h"
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

    static std::string DecorateTopCode(const std::string &str) {
        constexpr char kMainFuncTemplate[] = "main() {\n%1%\n}\nmain\n";
        std::string ret;
        ret =
            (boost::format(kMainFuncTemplate) % AddIndentToEachLine(str)).str();

        // Add built-in things
        ret = std::string() + kSushiUnitDef + '\n' + kSushiFuncRetDef + '\n' +
              kSushiFuncMapRetDef + '\n' + kSushiAbsFuncDef + '\n' +
              kSushiDupStrFuncDef + '\n' + kSushiPathConcatFuncDef + '\n' +
              kSushiFileEqFuncDef + '\n' + kSushiCompArrayFuncDef + '\n' + ret;
        return ret;
    }
};

} // namespace code_generation
} // namespace sushi

#endif
