#ifndef SUSHI_LEXER_LOOKAHEAD_STREAM_H_
#define SUSHI_LEXER_LOOKAHEAD_STREAM_H_

#include "boost/optional.hpp"
#include "sushi/util/meta.h"
#include <deque>
#include <string>
#include <type_traits>
#include <vector>

namespace sushi {
namespace lexer {
namespace detail {

template <typename T>
class LookaheadStream {
  public:
    using Chunk = sushi::util::if_t<
        std::is_same<T, char>::value, std::string, std::vector<T>>;

    virtual boost::optional<T> Next() {
        if (cache_.empty()) {
            return Consume();
        }
        T ret = std::move(cache_.front());
        cache_.pop_front();
        return std::move(ret);
    }

    Chunk Take(int n) {
        Chunk chunk;
        for (int i = 0; i < n; ++i) {
            auto next = Next();
            if (not next) {
                break;
            }
            chunk.push_back(*next);
        }
        return chunk;
    }

    template <typename P>
    Chunk TakeWhile(P p) {
        Chunk chunk;
        for (;;) {
            auto next = Lookahead();
            if (not next or not p(*next)) {
                break;
            }
            chunk.push_back(*next);
            Next();
        }
        return chunk;
    }

    boost::optional<const T &> Lookahead(int n = 1) {
        for (; cache_.size() < n;) {
            auto a = Consume();
            if (not a) {
                return boost::none;
            }
            cache_.emplace_back(std::move(*a));
        }
        return cache_[n - 1];
    }

    Chunk LookaheadMany(int n) {
        Chunk c;
        for (int i = 1; i <= n; ++i) {
            auto next = Lookahead(i);
            if (not next) {
                break;
            }
            c.push_back(*next);
        }
        return c;
    }

    template <typename P>
    void SkipWhile(P p) {
        for (auto n = Lookahead(); n and p(*n);) {
            Next();
        }
    }

    void Skip(int n) {
        for (int i = 0; i < n; ++i) {
            Next();
        }
    }

  private:
    virtual boost::optional<T> Consume() = 0;

    std::deque<T> cache_;
};

} // namespace detail
} // namespace lexer
} // namespace sushi

#endif // SUSHI_LEXER_LOOKAHEAD_STREAM_H_
