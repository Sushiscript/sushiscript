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
    // save some info about ident
    struct IdentInfo {
        // where define the ident
        TokenLocation defined_loc;
        // the ident's scope
        Scope *defined_scope;
        // bool is_const; // not useful currently
    };

    // save the outer scope
    Scope(std::shared_ptr<Scope> outer) : outer_(outer) {
        // initial bindings_
        bindings_ = Table<std::string, IdentInfo>();
    }

    // find the identinfo via name
    const Scope::IdentInfo *LookUp(const std::string &identifier) const {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_)) {
            // find in now bindings_
            return &iter->second;
        }
        if (outer_ == nullptr) {
            // no outer scope, return
            return nullptr;
        }
        // find in outer scope
        return (const Scope::IdentInfo *)(outer_->LookUp(identifier));
    }

    // find the identinfo via name and location (after location)
    const Scope::IdentInfo *
    LookUp(const std::string &identifier, const TokenLocation &use_loc) const {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_) and
            // this compare work becasue the location have overrided the '>'
            use_loc > iter->second.defined_loc) {
            return &iter->second;
        }
        if (outer_ == nullptr) {
            // no outer scope, return
            return nullptr;
        }
        // find in outer scope
        return (const Scope::IdentInfo *)(outer_->LookUp(identifier, use_loc));
    }

    bool Insert(const std::string &identifier, IdentInfo t) {
        if (bindings_.count(identifier)) {
            // the ident have be in the scope
            return false;
        }
        // insert
        bindings_.emplace(identifier, std::move(t));
        return true;
    }

    // return outer scope
    const Scope *Outer() const {
        return outer_.get();
    }

  private:
    // outer scope
    std::shared_ptr<Scope> outer_;
    // save name and identinfo's table
    Table<std::string, IdentInfo> bindings_;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_SCOPE_H_
