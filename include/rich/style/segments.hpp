/// @file segmentss.hpp
#pragma once
#include <list>
#include <string_view>
#include <fmt/color.h>
#include <fmt/ranges.h>

#include <rich/style/segment.hpp>

namespace rich {
  struct segments {
  private:
    std::list<segment> instance_{};

  public:
    segments() = default;
    explicit segments(std::string_view t) : instance_(1, segment(t)) {}
    segments(std::string_view t, const fmt::text_style& s)
      : instance_(1, segment(t, s)) {}

    // observer
    auto begin() const { return instance_.begin(); }
    auto end() const { return instance_.end(); }
    auto empty() const { return instance_.empty(); }
    auto size() const { return instance_.size(); }

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

    auto separate(const std::size_t offset) {
      auto [it, inner_pos] = partition_point(offset);
      if (inner_pos == 0)
        return it;
      assert(it != instance_.end());
      const auto old_segment = *it;
      // 指定した要素を消去し、消去した要素の次のイテレータを返す
      auto emplace_pos = instance_.erase(it);
      // 指定した要素の前に直接構築し、直接構築した要素のイテレータを返す
      emplace_pos = instance_.emplace(
        emplace_pos, old_segment.text().substr(inner_pos), old_segment.style());
      instance_.emplace(emplace_pos, old_segment.text().substr(0, inner_pos),
                        old_segment.style());
      // 構築した2要素のうち、後ろの要素を返す
      return emplace_pos;
    }

    auto set_style(std::string_view rng, const fmt::text_style& style) {
      assert(!instance_.empty());
      // イテレータを無効化させないため、後ろを先に分割する
      const auto offset2 = cast<std::size_t>(
        _ranges::distance(_ranges::front(instance_).text().begin(), rng.end()));
      auto last = separate(offset2);
      const auto offset1 = cast<std::size_t>(_ranges::distance(
        _ranges::front(instance_).text().begin(), rng.begin()));
      auto first = separate(offset1);
      for (; first != last; ++first)
        first->set_style(style);
      return first;
    }

    // friend
    // Friend declarations cannot refer to partial specializations
    // https://en.cppreference.com/w/cpp/language/friend
    friend struct fmt::formatter<segments>;
  };
} // namespace rich

template <>
struct fmt::formatter<rich::segments>
  : fmt::formatter<decltype(fmt::join(
      std::declval<const std::list<rich::segment>&>(), ""))> {
  template <typename FormatContext>
  auto format(const rich::segments& segs, FormatContext& ctx) const
    -> decltype(ctx.out()) {
    return fmt::formatter<decltype(fmt::join(
      std::declval<const std::list<rich::segment>&>(),
      ""))>::format(fmt::join(segs.instance_, ""), ctx);
  }
};
