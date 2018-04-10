#ifndef SUSHI_SCOPE_H_
#define SUSHI_SCOPE_H_

#include "sushi/type-system.h"
#include <memory>
#include <unordered_map>

namespace sushi {

class Scope {
  public:
    Scope(std::shared_ptr<Scope> outer) : outer_(outer) {}

    type::Type *LookUp(const std::string &identifier) {
        auto iter = bindings_.find(identifier);
        if (iter != end(bindings_)) {
            return iter->second.get();
        }
        if (outer_ == nullptr) {
            return nullptr;
        }
        return outer_->LookUp(identifier);
    }

    bool Insert(const std::string &identifier, std::unique_ptr<type::Type> t) {
        if (bindings_.count(identifier)) {
            return false;
        }
        bindings_[identifier] = std::move(t);
        return true;
    }

  private:
    std::shared_ptr<Scope> outer_;
    std::unordered_map<std::string, std::unique_ptr<type::Type>> bindings_;
};

} // namespace sushi

#endif
