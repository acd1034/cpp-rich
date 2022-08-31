/// @file enumerate.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/math.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/syntax.py#L262-L277
  template <line_formattable L>
  struct enumerate {
    using char_type = typename L::char_type;
    L contents{};
    std::size_t start_line = 1;
    std::size_t end_line = start_line;
    fmt::text_style number_highlight_style = {};
    format_spec<char_type> number_spec{
      .style = fmt::emphasis::faint,
      .fill = " ",
      .align = align_t::right,
      .width = 0,
    };
    std::size_t highlight_line = 0;
    std::basic_string_view<char_type> highlight_char = "❯";
    fmt::text_style highlight_style = fg(fmt::terminal_color::red);
    format_spec<char_type> highlight_spec{
      .style = number_spec.style,
      .fill = number_spec.fill,
      .align = align_t::left,
      .width = 0,
    };

    enumerate() = default;
    constexpr explicit enumerate(const L& l, int = {}) : contents(l) {}
    constexpr explicit enumerate(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  enumerate(R&&, int = {})
    -> enumerate<lines<typename std::ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
struct rich::line_formatter<rich::enumerate<L>, Char> {
private:
  const rich::enumerate<L>* ptr_ = nullptr;
  std::size_t current_ = 1;
  std::size_t nwidth_ = 0;
  line_formatter<L, Char> line_fmtr_;

  static constexpr std::size_t calculate_nwidth(const rich::enumerate<L>& l) {
    std::size_t w = std::max(l.start_line, l.end_line);
    w = ilog10(w) + 1;
    return std::max(w, l.number_spec.width);
  }

public:
  explicit line_formatter(const rich::enumerate<L>& l)
    : ptr_(std::addressof(l)), current_(l.start_line),
      nwidth_(calculate_nwidth(l)), line_fmtr_(l.contents) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and line_fmtr_;
  }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    const auto hlwidth = ptr_->highlight_spec.width;
    return sat_add(hlwidth + nwidth_ + 1, line_fmtr_.formatted_size());
  }

  template <std::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    const auto& ns = ptr_->number_spec;
    const auto& hs = ptr_->highlight_spec;
    const auto current = current_++;
    // clang-format off
    if (current == ptr_->highlight_line){
      const auto& c = ptr_->highlight_char;
      out = aligned_format_to<Char>(out, ptr_->highlight_style, c, hs.fill, hs.align, npos_sub(hs.width, !c.empty()));
      out = line_format_to<Char>(out, ptr_->number_highlight_style, current, ns.fill, ns.align, nwidth_);
    } else {
      out = spec_format_to<Char>(out, hs, "");
      out = line_format_to<Char>(out, ns.style, current, ns.fill, ns.align, nwidth_);
    }
    *out++ = ' ';
    auto result = line_fmtr_.format_to(out, npos_sub(n, hs.width + nwidth_ + 1));
    // clang-format on
    out = result.out;
    return {out, n};
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::enumerate<L>, Char>
  : rich::line_formattable_default_formatter<rich::enumerate<L>, Char> {};
