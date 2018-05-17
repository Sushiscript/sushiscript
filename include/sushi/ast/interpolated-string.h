#ifndef SUSHI_AST_INTERPOLATED_STRING_H_
#define SUSHI_AST_INTERPOLATED_STRING_H_

#include "./expression/expression.h"
#include <functional>
#include <memory>
#include <string>

namespace sushi {
namespace ast {

class InterpolatedString {
  public:
    struct InterpolatedNode {

        virtual ~InterpolatedNode() {}

        void Traverse(
            std::function<void(const std::string &s)> fs,
            std::function<void(const Expression &)> fe) const {
            Visit(fs, fe);
            if (next != nullptr) {
                next->Traverse(std::move(fs), std::move(fe));
            }
        }
        std::unique_ptr<InterpolatedNode> next;

        virtual void Visit(
            std::function<void(const std::string &)> &,
            std::function<void(const Expression &)> &) const = 0;
    };

    void Append(std::string s) {
        AppendNode(std::make_unique<PlainString>(std::move(s)));
    };

    void Append(std::unique_ptr<Expression> expr) {
        AppendNode(std::make_unique<Interpolation>(std::move(expr)));
    }

    void Traverse(
        std::function<void(const std::string &)> fs,
        std::function<void(const Expression &)> fe) const {
        if (node_ != nullptr) {
            node_->Traverse(std::move(fs), std::move(fe));
        }

    }

  private:
    struct PlainString : public InterpolatedNode {
        PlainString(std::string s) : s(std::move(s)) {}

      private:
        virtual void Visit(
            std::function<void(const std::string &)> &fs,
            std::function<void(const Expression &)> &) const override {
            fs(s);
        }

        std::string s;
    };

    struct Interpolation : public InterpolatedNode {
        Interpolation(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
        std::unique_ptr<Expression> expr;

      private:
        virtual void Visit(
            std::function<void(const std::string &)> &,
            std::function<void(const Expression &)> &fe) const override {
            fe(*expr);
        };
    };

    void AppendNode(std::unique_ptr<InterpolatedNode> next) {
        if (node_ == nullptr) {
            node_ = std::move(next);
            tail_ = node_.get();
        } else {
            tail_->next = std::move(next);
            tail_ = tail_->next.get();
        }
    }
    std::unique_ptr<InterpolatedNode> node_;
    InterpolatedNode *tail_;
};

} // namespace ast
} // namespace sushi

#endif // SUSHI_AST_INTERPOLATED_STRING_H_
