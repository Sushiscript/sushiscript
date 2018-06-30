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

    // create new identinfo with boost::optional<TokenLocation> and Scope*
    static IdentInfo CreateIdentInfo(
        boost::optional<TokenLocation> start_location, Scope *defined_scope);

    Scope(std::shared_ptr<Scope> outer);
    const Scope *Outer() const;
    const IdentInfo *LookUp(const std::string &identifier) const;
    const IdentInfo *
    LookUp(const std::string &identifier, const TokenLocation &use_loc) const;
    bool Insert(const std::string &identifier, IdentInfo t);

  private:
    // outer scope
    std::shared_ptr<Scope> outer_;
    // save name and identinfo's table
    Table<std::string, IdentInfo> bindings_;
};

} // namespace scope
} // namespace sushi

#endif // SUSHI_SCOPE_SCOPE_H_
