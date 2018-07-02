#include "sushi/code-generation/code-generation.h"
#include "sushi/code-generation/stmt-visitor.h"
#include "sushi/code-generation/util.h"

namespace sushi {
namespace code_generation {

std::string CodeGenerator::GenCode(
    const ast::Program &program, const scope::Environment &environment,
    std::shared_ptr<ScopeManager> scope_manager) {
    // TODO: The following rule may change in the future
    // scope_manager == nullptr means that this is the top program
    bool is_top_program = (scope_manager == nullptr);

    if (!scope_manager) {
        scope_manager = std::make_shared<ScopeManager>();
    }

    std::unordered_set<std::string> new_ids;

    std::string ret;
    for (auto &statement : program.statements) {
        StmtVisitor visitor(environment, program, scope_manager);
        statement->AcceptVisitor(visitor);
        // new_ids.merge(visitor.new_ids);
        MergeSets(new_ids, visitor.new_ids);
        ret += visitor.code + "\n";
    }

    // unset new_ids
    constexpr char unset_template[] = "unset %1%";
    for (auto &id : new_ids) {
        scope_manager->Unset(id);
        ret += '\n' + (boost::format(unset_template) % id).str();
    }

    if (is_top_program) {
        constexpr char kMainFuncTemplate[] = "main() {\n%1%\n}\nmain\n";
        ret = (boost::format(kMainFuncTemplate) % AddIndentToEachLine(ret)).str();
    }

    return ret;
}

} // namespace code_generation
} // namespace sushi
