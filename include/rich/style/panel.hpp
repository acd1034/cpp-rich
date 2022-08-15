/// @file panel.hpp
#pragma once
#include <string_view>
#include <vector>

#include <rich/format.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/panel.py#L38-L53
  template <class L>
  struct panel {
    L lines{};
    fmt::text_style style{};
    std::size_t width = 60;

    panel() = default;
    constexpr explicit panel(const L& l, fmt::text_style s = {})
      : lines(l), style(s) {}
    constexpr explicit panel(L&& l, fmt::text_style s = {})
      : lines(std::move(l)), style(s) {}
  };

  template <line_range R>
  panel(R&&, fmt::text_style = {})
    -> panel<lines<typename ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <typename L, typename Char>
requires rich::line_formattable<L, Char>
struct rich::line_formatter<rich::panel<L>, Char> {
private:
  const rich::panel<L>* ptr_ = nullptr;
  std::uint32_t phase_ = 0;
  line_formatter<L, Char> line_fmtr_;

public:
  explicit line_formatter(const rich::panel<L>& l)
    : ptr_(std::addressof(l)), line_fmtr_(l.lines) {}

  operator bool() const { return ptr_ != nullptr and phase_ != 2; }
  bool operator!() const { return !bool(*this); }

  template <ranges::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    const auto w = std::min(ptr_->width, n);
    assert(w != line_formatter_npos);
    switch (phase_) {
    case 0:
      ++phase_;
      return {fmt::format_to(out, ptr_->style, "╭{0:─<{1}}╮", "", w - 2), w};
    case 1: {
      if (!line_fmtr_) {
        ++phase_;
        return {fmt::format_to(out, ptr_->style, "╰{0:─<{1}}╯", "", w - 2), w};
      }
      out = fmt::format_to(out, ptr_->style, "│ ");
      auto result = line_fmtr_.format_to(out, w - 4);
      out = result.out;
      if (result.size < w - 4)
        fmt::format_to(out, "{0: <{1}}", "", w - 4 - result.size);
      return {fmt::format_to(out, ptr_->style, " │"), w};
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::panel<L>, Char>
  : rich::line_formattable_default_formatter<rich::panel<L>, Char> {};
