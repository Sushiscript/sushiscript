#ifndef SUSHI_SCOPE_ENVIRMENT_H_
#define SUSHI_SCOPE_ENVIRMENT_H_

#include "sushi/ast/expression/identifier.h"
#include "sushi/ast/statement.h"
#include "sushi/lexer/token-location.h"
#include "sushi/type-system.h"
#include "scope.h"
#include <memory>
#include <unordered_map>

namespace sushi {
namespace scope {


class Environment {
  public:
    Environment(ast::Program program) {
        for (auto & statement : program.statements) {
            // pass
        }
    }
    bool
    Insert(const ast::Identifier *ident, std::shared_ptr<Scope> def_scope) {
        if (idents_.count(ident)) {
            return false;
        }
        idents_[ident] = def_scope;
        return true;
    }
    bool Insert(const ast::Program *block, std::shared_ptr<Scope> scope) {
        if (blocks_.count(block)) {
            return false;
        }
        blocks_[block] = scope;
        return true;
    }
    bool Insert(const ast::Expression *expr, std::unique_ptr<type::Type> t) {
        if (typings_.count(expr)) {
            return false;
        }
        typings_[expr] = std::move(t);
        return true;
    }
    const Scope *LookUp(const ast::Identifier *ident) const {
        auto iter = idents_.find(ident);
        return iter == end(idents_) ? nullptr : iter->second.get();
    }
    const Scope *LookUp(const ast::Program *block) const {
        auto iter = blocks_.find(block);
        return iter == end(blocks_) ? nullptr : iter->second.get();
    }
    const type::Type *LookUp(const ast::Expression *expr) const {
        auto iter = typings_.find(expr);
        return iter == end(typings_) ? nullptr : iter->second.get();
    }

  private:
    std::unordered_map<const ast::Identifier *, std::shared_ptr<Scope>> idents_;
    std::unordered_map<const ast::Program *, std::shared_ptr<Scope>> blocks_;
    std::unordered_map<const ast::Expression *, std::unique_ptr<type::Type>>
        typings_;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_ENVIRMENT_H_
