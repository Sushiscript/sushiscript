#ifndef SUSHI_SCOPE_ENVIRONMENT_H_
#define SUSHI_SCOPE_ENVIRONMENT_H_

#include "scope.h"
#include "sushi/ast.h"
#include "sushi/lexer.h"
#include "sushi/type-system/type.h"
#include "util.h"
#include <memory>
#include <unordered_map>

namespace sushi {
namespace scope {

class Environment {
  public:
    Environment();
    bool Insert(const ast::Identifier *ident, std::shared_ptr<Scope> def_scope);
    bool Insert(const ast::Program *block, std::shared_ptr<Scope> scope);
    bool Insert(const ast::Expression *expr, std::unique_ptr<type::Type> t);
    const Scope *LookUp(const ast::Identifier *ident) const;
    const Scope *LookUp(const ast::Program *block) const;
    const type::Type *LookUp(const ast::Expression *expr) const;

  // private:
    Table<const ast::Identifier *, std::shared_ptr<Scope>> idents_;
    Table<const ast::Program *, std::shared_ptr<Scope>> blocks_;
    Table<const ast::Expression *, std::unique_ptr<type::Type>> typings_;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_ENVIRONMENT_H_
