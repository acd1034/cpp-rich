/// @file segments.hpp
#pragma once
#include <list>
#include <string_view>

#include <rich/exception.hpp>
#include <rich/format.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  template <typename Char>
  struct segments {
  private:
    using string_view_type = std::basic_string_view<Char>;
    std::list<segment<Char>> instance_{};

  public:
    using char_type = Char;

    // ctor
    segments() = default;
    explicit segments(string_view_type t) : instance_(1, segment(t)) {}
    segments(string_view_type t, const fmt::text_style& s)
      : instance_(1, segment(t, s)) {}

    // observer
    auto begin() const { return instance_.begin(); }
    auto end() const { return instance_.end(); }
    auto empty() const { return instance_.empty(); }
    auto size() const { return instance_.size(); }

    // operation
    // NOTE: `*this` is mutable
    auto partition_point(const std::size_t offset) {
      std::size_t current = 0;
      auto end = instance_.end();
      for (auto it = instance_.begin(); it != end; ++it) {
        const std::size_t next = current + it->text().size();
        if (next > offset)
          return std::make_pair(it, offset - current);
        current = next;
      }
      return std::make_pair(end, cast<std::size_t>(0));
    }

    // NOTE: returned iterator is not const_iterator
    auto split(const std::size_t offset) {
      auto [it, inner_pos] = partition_point(offset);
      if (inner_pos == 0)
        return it;
      assert(it != instance_.end());
      const auto old_text = it->text();
      it->text() = old_text.substr(inner_pos);
      // 指定した要素の前に直接構築し、直接構築した要素のイテレータを返す
      instance_.emplace(it, old_text.substr(0, inner_pos), it->style());
      // 構築した2要素のうち、後ろの要素を返す
      return it;
    }

    auto set_style(string_view_type rng, const fmt::text_style& style) {
      if (instance_.empty())
        throw runtime_error("`segments` not initialized");
      // イテレータを無効化させないため、後ろを先に分割する
      const auto offset2 = cast<std::size_t>(
        ranges::distance(ranges::front(instance_).text().begin(), rng.end()));
      auto last = split(offset2);
      const auto offset1 = cast<std::size_t>(ranges::distance(
        ranges::front(instance_).text().begin(), rng.begin()));
      auto first = split(offset1);
      for (; first != last; ++first)
        first->style() = style;
      return first;
    }

    auto add_style(string_view_type rng, const fmt::text_style& style) {
      if (instance_.empty())
        throw runtime_error("`segments` not initialized");
      // イテレータを無効化させないため、後ろを先に分割する
      const auto offset2 = cast<std::size_t>(
        ranges::distance(ranges::front(instance_).text().begin(), rng.end()));
      auto last = split(offset2);
      const auto offset1 = cast<std::size_t>(ranges::distance(
        ranges::front(instance_).text().begin(), rng.begin()));
      auto first = split(offset1);
      for (; first != last; ++first)
        // スタイルが重複したときに例外を投げる
        first->style() |= style;
      return first;
    }
  };
} // namespace rich

template <typename Char>
struct fmt::formatter<rich::segments<Char>, Char>
  : fmt::formatter<
      fmt::join_view<ranges::iterator_t<rich::segments<Char>>,
                     ranges::sentinel_t<rich::segments<Char>>, Char>,
      Char> {
  template <typename FormatContext>
  auto format(const rich::segments<Char>& segs, FormatContext& ctx) const
    -> decltype(ctx.out()) {
    using base_type = fmt::formatter<
      fmt::join_view<ranges::iterator_t<rich::segments<Char>>,
                     ranges::sentinel_t<rich::segments<Char>>, Char>,
      Char>;
    return base_type::format(fmt::join(segs, ""), ctx);
  }
};
