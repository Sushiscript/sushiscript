#include "sushi/scope.h"

namespace sushi {
namespace scope {

Environment::Environment() {
    // initial the table
    idents_ = Table<const ast::Identifier *, std::shared_ptr<Scope>>();
    blocks_ = Table<const ast::Program *, std::shared_ptr<Scope>>();
    typings_ = Table<const ast::Expression *, std::unique_ptr<type::Type>>();
}

bool Environment::Insert(
    const ast::Identifier *ident, std::shared_ptr<Scope> def_scope) {
    if (idents_.count(ident)) {
        return false;
    }
    idents_[ident] = def_scope;
    return true;
}

bool Environment::Insert(
    const ast::Program *block, std::shared_ptr<Scope> scope) {
    if (blocks_.count(block)) {
        return false;
    }
    blocks_[block] = scope;
    return true;
}

bool Environment::Insert(
    const ast::Expression *expr, std::unique_ptr<type::Type> t) {
    if (typings_.count(expr)) {
        return false;
    }
    typings_[expr] = std::move(t);
    return true;
}

const Scope *Environment::LookUp(const ast::Identifier *ident) const {
    auto iter = idents_.find(ident);
    return iter == end(idents_) ? nullptr : iter->second.get();
}

const Scope *Environment::LookUp(const ast::Program *block) const {
    auto iter = blocks_.find(block);
    return iter == end(blocks_) ? nullptr : iter->second.get();
}

const type::Type *Environment::LookUp(const ast::Expression *expr) const {
    auto iter = typings_.find(expr);
    return iter == end(typings_) ? nullptr : iter->second.get();
    return nullptr;
}

} // namespace scope
} // namespace sushi
