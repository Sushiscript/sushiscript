#ifndef SUSHI_SCOPE_H_
#define SUSHI_SCOPE_H_

#include "sushi/ast/expression/identifier.h"
#include "sushi/ast/statement.h"
#include "sushi/lexer/token-location.h"
#include "sushi/type-system.h"
#include <memory>
#include <unordered_map>

namespace sushi {

/// ownership: (=> means `own`)
///
/// Environment => Scope
/// Scope => IdentInfo
/// Scope => Scope (outer scopes)

class Scope {
  public:
    struct IdentInfo {
        TokenLocation defined_loc;
        Scope *defined_scope;
        // bool is_const; // not useful currently
    };

    Scope(std::shared_ptr<Scope> outer) : outer_(outer) {}

    const Scope::IdentInfo *LookUp(const std::string &identifier) const {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_)) {
            return &iter->second;
        }
        if (outer_ == nullptr) {
            return nullptr;
        }
        return outer_->LookUp(identifier);
    }

    const Scope::IdentInfo *
    LookUp(const std::string &identifier, const TokenLocation &use_loc) const {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_) and use_loc > iter->second.defined_loc) {
            return &iter->second;
        }
        if (outer_ == nullptr) {
            return nullptr;
        }
        return outer_->LookUp(identifier, use_loc);
    }

    bool Insert(const std::string &identifier, IdentInfo t) {
        if (bindings_.count(identifier)) {
            return false;
        }
        bindings_.emplace(identifier, std::move(t));
        return true;
    }

    const Scope *Outer() const {
        return outer_.get();
    }

  private:
    std::shared_ptr<Scope> outer_;
    std::unordered_map<std::string, IdentInfo> bindings_;
};

class Environment {
  public:
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

} // namespace sushi

#endif
