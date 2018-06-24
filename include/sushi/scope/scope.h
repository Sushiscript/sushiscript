#ifndef SUSHI_SCOPE_SCOPE_H_
#define SUSHI_SCOPE_SCOPE_H_

#include "sushi/ast.h"
#include "sushi/lexer.h"
#include "sushi/type-system.h"
#include "util.h"
#include <memory>
#include <unordered_map>

namespace sushi {
namespace scope {

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

    Scope(std::shared_ptr<Scope> outer) : outer_(outer) {
        bindings_ = Table<std::string, IdentInfo>();
    }

    const Scope::IdentInfo *LookUp(const std::string &identifier) const {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_)) {
            return &iter->second;
        }
        if (outer_ == nullptr) {
            return nullptr;
        }
        return (const Scope::IdentInfo *)(outer_->LookUp(identifier));
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
        return (const Scope::IdentInfo *)(outer_->LookUp(identifier, use_loc));
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
    Table<std::string, IdentInfo> bindings_;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_SCOPE_H_
