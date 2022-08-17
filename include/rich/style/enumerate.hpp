/// @file enumerate.hpp
#pragma once
#include <array>
#include <charconv>

#include <rich/format.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>
#include <rich/saturation.hpp>

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/syntax.py#L262-L277
  template <class L>
  struct enumerate {
    using char_type = typename L::char_type;
    L contents{};
    std::size_t start_line = 1;
    format_spec<char_type> number_spec{
      .style = fmt::emphasis::faint,
      .fill = " ",
      .align = align_t::right,
      .width = 4,
    };
    // format_spec<char_type> highlight_spec{
    //   .style = fg(fmt::terminal_color::red),
    //   .fill = box[4],
    //   .align = align_t::left,
    //   .width = 2,
    // };

    enumerate() = default;
    constexpr explicit enumerate(const L& l, int = {}) : contents(l) {}
    constexpr explicit enumerate(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  enumerate(R&&, int = {})
    -> enumerate<lines<typename ranges::range_value_t<R>::char_type>>;

  auto _to_chars(const std::size_t n) {
    static std::array<char, 20> a{};
    auto first = a.data();
    auto [ptr, ec] = std::to_chars(first, first + a.size(), n);
    assert(ec == std::errc{});
    return std::string_view(first, ptr);
  }
} // namespace rich

template <typename L, typename Char>
requires rich::line_formattable<L, Char>
struct rich::line_formatter<rich::enumerate<L>, Char> {
private:
  const rich::enumerate<L>* ptr_ = nullptr;
  std::size_t current_ = 1;
  line_formatter<L, Char> line_fmtr_;

public:
  explicit line_formatter(const rich::enumerate<L>& l)
    : ptr_(std::addressof(l)), current_(l.start_line), line_fmtr_(l.contents) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and line_fmtr_;
  }
  constexpr bool operator!() const { return !bool(*this); }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    return sat_add(ptr_->number_spec.width + 1, line_fmtr_.formatted_size());
  }

  template <ranges::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    const auto& ns = ptr_->number_spec;
    auto sv = _to_chars(current_++);
    // clang-format off
    out = aligned_format_to<Char>(out, ns.style, sv, ns.fill, ns.align, npos_sub(ns.width, sv.size()));
    *out++ = ' ';
    auto result = line_fmtr_.format_to(out, sat_sub(n, ns.width + 1));
    // clang-format on
    return {result.out, sat_add(ns.width + 1, result.size)};
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::enumerate<L>, Char>
  : rich::line_formattable_default_formatter<rich::enumerate<L>, Char> {};
