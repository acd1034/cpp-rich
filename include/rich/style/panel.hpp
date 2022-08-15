/// @file panel.hpp
#pragma once
#include <string_view>
#include <vector>

#include <rich/format.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/panel.py#L38-L53
  template <class L>
  struct panel {
    using char_type = typename L::char_type;
    L lines{};
    std::size_t width = 80;
    format_spec<char_type> boarder_spec{
      .fill = {},  // ignored
      .align = {}, // ignored
      .width = 2,
      .style = fg(fmt::terminal_color::red),
    };
    std::basic_string_view<char_type> title{};

    panel() = default;
    constexpr explicit panel(const L& l, int = {}) : lines(l) {}
    constexpr explicit panel(L&& l, int = {}) : lines(std::move(l)) {}
  };

  template <line_range R>
  panel(R&&, int = {})
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
    const auto& bs = ptr_->boarder_spec;
    assert(bs.width * 2 < w and w < line_formatter_npos);

    switch (phase_) {
    case 0: {
      ++phase_;
      // clang-format off
      out = fmt::format_to(out, bs.style, "{:─<{}}", "╭", bs.width);
      out = fmt::format_to(out, bs.style, "{:─^{}}", ptr_->title, w - bs.width * 2);
      out = fmt::format_to(out, bs.style, "{:─>{}}", "╮", bs.width);
      // clang-format on
      return {out, w};
    }
    case 1: {
      if (!line_fmtr_) {
        ++phase_;
        out = fmt::format_to(out, bs.style, "{:─<{}}", "╰", bs.width);
        out = fmt::format_to(out, bs.style, "{:─<{}}", "", w - bs.width * 2);
        out = fmt::format_to(out, bs.style, "{:─>{}}", "╯", bs.width);
        return {out, w};
      }

      out = fmt::format_to(out, bs.style, "{: <{}}", "│", bs.width);
      const auto w2 = w - bs.width * 2;
      auto result = line_fmtr_.format_to(out, w2);
      out = result.out;
      out = fmt::format_to(out, "{: <{}}", "", w2 - result.size);
      out = fmt::format_to(out, bs.style, "{: >{}}", "│", bs.width);
      return {out, w};
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::panel<L>, Char>
  : rich::line_formattable_default_formatter<rich::panel<L>, Char> {};
