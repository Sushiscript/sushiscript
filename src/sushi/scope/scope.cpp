#include "sushi/scope.h"

namespace sushi {
namespace scope {

// create new identinfo with boost::optional<TokenLocation> and Scope*
Scope::IdentInfo Scope::CreateIdentInfo(
    boost::optional<TokenLocation> start_location, Scope *defined_scope) {
    if (!start_location.is_initialized()) {
        // todo: add error process system
        // debug::log(start_location,
        //     Error::ToString(Error::Type::kNoDefineLocationError));
        auto info = Scope::IdentInfo{};
        info.defined_scope = defined_scope;
        return info;
    } else {
        return Scope::IdentInfo{start_location.get(), defined_scope};
    }
}

// save the outer scope
Scope::Scope(std::shared_ptr<Scope> outer) : outer_(outer) {
    // initial bindings_
    bindings_ = Table<std::string, IdentInfo>();
}

// return outer scope
const Scope *Scope::Outer() const {
    return outer_.get();
}

// find the identinfo via name
const Scope::IdentInfo *Scope::LookUp(const std::string &identifier) const {
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
const Scope::IdentInfo *Scope::LookUp(
    const std::string &identifier, const TokenLocation &use_loc) const {
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

bool Scope::Insert(const std::string &identifier, IdentInfo t) {
    if (bindings_.count(identifier)) {
        // the ident have be in the scope
        return false;
    }
    // insert
    bindings_.emplace(identifier, std::move(t));
    return true;
}

} // namespace scope
} // namespace sushi
