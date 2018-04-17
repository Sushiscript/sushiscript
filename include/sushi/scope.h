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
        std::unique_ptr<type::Type> type;
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
    LookUp(const std::string &identifier, const TokenLocation &use_loc) {
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
        bindings_[identifier] = std::move(t);
        return true;
    }

  private:
    std::shared_ptr<Scope> outer_;
    std::unordered_map<std::string, IdentInfo> bindings_;
};

class Environment {
  public:
    struct BlockInfo {
        std::shared_ptr<Scope> scope;
    };

  private:
    std::unordered_map<const ast::Identifier *, std::shared_ptr<Scope>> idents_;
    std::unordered_map<const ast::Program *, Environment::BlockInfo> blocks_;
};

} // namespace sushi

#endif
