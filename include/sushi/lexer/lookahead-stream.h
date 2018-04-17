#ifndef SUSHI_LEXER_LOOKAHEAD_STREAM_H_
#define SUSHI_LEXER_LOOKAHEAD_STREAM_H_

#include "boost/optional.hpp"
#include <deque>

namespace sushi {

template <typename T>
class LookaheadStream {
  public:
    virtual boost::optional<T> Next() {
        if (cache_.empty()) {
            return Consume();
        }
        T ret = std::move(cache_.front());
        cache_.pop_front();
        return std::move(ret);
    }

    boost::optional<const T &> Lookahead(int n = 0) {
        for (; cache_.size() <= n;) {
            auto a = Consume();
            if (not a) {
                return boost::none;
            }
            cache_.emplace_back(std::move(*a));
        }
        return cache_[n];
    }

    template <typename P>
    void Skip(P p) {
        for (auto n = Lookahead(); n and p(*n);)
            Next();
    }

  private:
    virtual boost::optional<T> Consume() = 0;

    std::deque<T> cache_;
};

} // namespace sushi

#endif // SUSHI_LEXER_LOOKAHEAD_STREAM_H_
