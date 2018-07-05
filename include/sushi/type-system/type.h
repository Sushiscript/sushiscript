#ifndef SUSHI_TYPE_SYSTEM_TYPE_H_
#define SUSHI_TYPE_SYSTEM_TYPE_H_

#include "sushi/util/visitor.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace sushi {
namespace type {

struct Simple;
struct Array;
struct Map;
struct Function;

using TypeVisitor =
    sushi::util::DefineVisitor<Simple, Array, Map, Function>;

// highly simplified version of type that currently support only built-in types
struct Type {
    SUSHI_VISITABLE(TypeVisitor)

    using Pointer = std::unique_ptr<Type>;

    virtual const Simple *ToSimple() const {
        return nullptr;
    }
    virtual const Array *ToArray() const {
        return nullptr;
    }
    virtual const Map *ToMap() const {
        return nullptr;
    }
    virtual const Function *ToFunction() const {
        return nullptr;
    }
    virtual bool Equals(const Type *) const = 0;

    virtual Pointer Copy() const = 0;

    virtual std::string ToString() const = 0;

    static bool ImplicitConvertible(const Type *from, const Type *to);

    std::string ToAtomString() const {
        std::string t = ToString();
        if (ToSimple()) return t;
        return "(" + t + ")";
    }

    bool Equals(const Pointer &rhs) {
        return Equals(rhs.get());
    }

    virtual ~Type() = default;
};

struct Simple : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)

    enum class Type {
        kInt,
        kBool,
        kUnit,
        kFd,
        kExitCode,
        kPath,
        kRelPath,
        kString,
        kChar
    };

    static std::string ToString(Type t) {
        using T = Type;
        switch (t) {
        case T::kInt: return "Int";
        case T::kBool: return "Bool";
        case T::kUnit: return "()";
        case T::kFd: return "FD";
        case T::kExitCode: return "ExitCode";
        case T::kPath: return "Path";
        case T::kRelPath: return "RelPath";
        case T::kString: return "String";
        case T::kChar: return "Char";
        }
    }

    const Simple *ToSimple() const override {
        return this;
    }

    static Pointer Make(Type t) {
        return std::make_unique<Simple>(t);
    }

    bool Equals(const sushi::type::Type *rhs) const override {
        auto b = rhs->ToSimple();
        if (not b) return false;
        return type == b->type;
    }

    Pointer Copy() const override {
        return Make(type);
    }

    std::string ToString() const override {
        return ToString(type);
    }

    Simple(Simple::Type type) : type(type) {}

    Simple::Type type;
};

struct Array : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)

    Array(Simple::Type element) : element(element) {}

    const Array *ToArray() const override {
        return this;
    }
    static Pointer Make(Simple::Type t) {
        return std::make_unique<Array>(t);
    }

    bool Equals(const Type *rhs) const override {
        auto a = rhs->ToArray();
        if (not a) return false;
        return element == a->element;
    }
    Pointer Copy() const override {
        return Make(element);
    }

    std::string ToString() const override {
        return "Array " + Simple::ToString(element);
    }

    // std::unique_ptr<Type> element;
    Simple::Type element;
};

struct Map : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type)

    Map(Simple::Type key, Simple::Type value)
        : key(key), value(value) {}

    const Map *ToMap() const override {
        return this;
    }
    static Pointer Make(Simple::Type k, Simple::Type v) {
        return std::make_unique<Map>(k, v);
    }
    std::string ToString() const override {
        return "Map " + Simple::ToString(key) + " " +
               Simple::ToString(value);
    }

    bool Equals(const Type *rhs) const override {
        auto m = rhs->ToMap();
        if (not m) return false;
        return key == m->key and value == m->value;
    }
    Pointer Copy() const override {
        return Make(key, value);
    }

    Simple::Type key;
    // std::unique_ptr<Type> value;
    Simple::Type value;
};

struct Function : Type {
    SUSHI_ACCEPT_VISITOR_FROM(Type);

    Function(std::vector<Pointer> params, Pointer result)
        : params(std::move(params)), result(std::move(result)) {}

    const Function *ToFunction() const override {
        return this;
    }
    static Pointer Make(std::vector<Pointer> params, Pointer result) {
        return std::make_unique<Function>(std::move(params), std::move(result));
    }
    std::string ToString() const override {
        std::string res = "Function " + result->ToAtomString();
        for (auto &p : params) {
            res += " " + p->ToAtomString();
        }
        return res;
    }
    bool Equals(const Type *rhs) const override {
        auto f = rhs->ToFunction();
        if (not f) return false;
        if (f->params.size() != params.size()) return false;
        for (int i = 0; i < params.size(); ++i) {
            if (not params[i]->Equals(f->params[i].get())) return false;
        }
        return result->Equals(f->result.get());
    }
    Pointer Copy() const override {
        std::vector<Pointer> param_copies(params.size());
        Pointer ret_copy = result->Copy();
        std::transform(
            begin(params), end(params), begin(param_copies),
            [](auto &p) { return p->Copy(); });
        return Make(std::move(param_copies), std::move(ret_copy));
    }

    std::vector<Pointer> params;
    Pointer result;
};

inline bool Type::ImplicitConvertible(const Type *from, const Type *to) {
    if (from->Equals(to)) return true;

    auto simple_from = from->ToSimple(), simple_to = to->ToSimple();
    if (not(simple_from and simple_to)) return false;

    using T = Simple::Type;

    auto from_type = simple_from->type, to_type = simple_to->type;
    if (from_type == T::kExitCode and
        (to_type == T::kBool or to_type == T::kInt))
        return true;
    if (from_type == T::kRelPath and to_type == T::kPath) return true;
    return false;
}

} // namespace type
} // namespace sushi

#endif // SUSHI_TYPE_SYSTEM_TYPE_H_
