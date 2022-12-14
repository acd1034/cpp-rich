/// @file line_formatter.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/math.hpp>

namespace rich {

  // line_formatter
  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/format/formatter.h#L35-L40

  template <class T, class Char>
  struct line_formatter {
    line_formatter() = delete;
    line_formatter(const line_formatter&) = delete;
    line_formatter& operator=(const line_formatter&) = delete;
  };

  // line_formattable
  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/__format/concepts.h#L39-L47

  template <class Char>
  using fmt_iter_for = Char*;

  // clang-format off
  template <class T>
  concept line_formattable_impl =
    std::copyable<line_formatter<T, typename T::char_type>>
    and std::constructible_from<line_formatter<T, typename T::char_type>, const T&>
    and contextually_convertible_to_bool<line_formatter<T, typename T::char_type>>
    and requires(line_formatter<T, typename T::char_type> f,
                 fmt_iter_for<typename T::char_type> out,
                 const std::size_t n) {
    { std::as_const(f).formatted_size() } -> std::same_as<std::size_t>;
    { f.format_to(out, n) } -> std::same_as<fmt_iter_for<typename T::char_type>>;
  };
  // clang-format on

  template <class T>
  concept line_formattable = line_formattable_impl<std::remove_cvref_t<T>>;

  // line_formattable_default_formatter

  template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
  struct line_formattable_default_formatter {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const L& l, FormatContext& ctx) const -> decltype(ctx.out()) {
      auto out = ctx.out();
      char dlm = '\0';
      for (line_formatter<L, Char> line_fmtr(l); bool(line_fmtr);) {
        out = fmt::detail::write(out, std::exchange(dlm, '\n'));
        out = line_fmtr.format_to(out);
      }
      return out;
    }
  };

  // line_format_to

  inline constexpr auto line_formatter_npos = std::size_t(-1);

  std::size_t npos_sub(std::size_t x, std::size_t y) noexcept {
    if (x == line_formatter_npos) {
      assert(y != line_formatter_npos);
      return x;
    }
    return sat_sub(x, y);
  }

  template <typename Char, std::output_iterator<const Char&> Out, class L>
  Out line_format_to(Out out, const fmt::text_style& style,
                     line_formatter<L, Char>& line_fmtr,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    if (width == line_formatter_npos)
      return line_fmtr.format_to(out);

    const auto fillwidth = sat_sub(width, line_fmtr.formatted_size());
    const auto [left, right] = padding_size(align, fillwidth);
    out = padded_format_to<Char>(out, style, "", fill, 0, left);
    out = line_fmtr.format_to(out, width);
    out = padded_format_to<Char>(out, style, "", fill, 0, right);
    return out;
  }

  template <typename Char, std::output_iterator<const Char&> Out>
  Out line_format_to(Out out, const fmt::text_style& style,
                     std::basic_string_view<Char> sv,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    if (width == line_formatter_npos)
      return padded_format_to<Char>(out, style, sv, fill, 0, 0);

    sv = sv.substr(0, width);
    const auto fillwidth = width - sv.size();
    return aligned_format_to<Char>(out, style, sv, fill, align, fillwidth);
  }

  template <typename Char, std::output_iterator<const Char&> Out, class T>
  Out line_format_to(Out out, const fmt::text_style& style, const T& t,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    const auto str = fmt::format("{}", t);
    return line_format_to(out, style, std::basic_string_view<Char>(str), fill,
                          align, width);
  }
} // namespace rich
