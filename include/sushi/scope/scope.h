#ifndef SUSHI_SCOPE_SCOPE_H_
#define SUSHI_SCOPE_SCOPE_H_

#include "sushi/ast/expression/identifier.h"
#include "sushi/ast/statement.h"
#include "sushi/lexer/token-location.h"
#include "sushi/type-system.h"
#include <memory>
#include <unordered_map>

namespace sushi {
namespace scope {



/// ownership: (=> means `own`)
///
/// Environment => Scope
/// Scope => IdentInfo
/// Scope => Scope (outer scopes)

/// what scope do ? 
/// scope part is between the lexer part and code-generation part, it will 
/// receive infomation from lexer part and offer some useful services to 
/// code-generation part, in detail:
/// 1.type-check part, as far as I know, now we don't do the type check works.
/// 2.find-scope part, help to find {a. identifier, b. expression, c. program}'s
/// scope. 


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

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_SCOPE_H_
