/// @file format.hpp
#pragma once
#include <fmt/color.h>

#include <rich/fundamental.hpp>

namespace rich {

  // RICH_TYPED_LITERAL
  // See https://github.com/microsoft/STL/blob/17fde2cbab6e8724d81c9555237c9a623d7fb954/tests/std/tests/P0220R1_string_view/test.cpp#L260-L277

  template <class Char>
  struct choose_literal; // not defined

  template <>
  struct choose_literal<char> {
    static constexpr const char* choose(const char* s, const wchar_t*) {
      return s;
    }
  };

  template <>
  struct choose_literal<wchar_t> {
    static constexpr const wchar_t* choose(const char*, const wchar_t* s) {
      return s;
    }
  };

#define RICH_TYPED_LITERAL(Char, Literal)                                      \
  (rich::choose_literal<Char>::choose(Literal, L##Literal))

  // copy_to
  // https://github.com/fmtlib/fmt/blob/fd41110d383b7240231718f009b21498e3984ccc/include/fmt/core.h#L838-L853

  template <class Out, class In, class Size = std::size_t>
  constexpr Out copy_to(Out out, In first, In last, Size n = 1) {
    while (n--)
      for (In in = first; in != last;)
        *out++ = *in++;
    return out;
  }

  template <class T, class Size = std::size_t>
  constexpr T* copy_to(T* out, const T* first, const T* last, Size n = 1) {
    if (std::is_constant_evaluated())
      return copy_to<T*, const T*, Size>(out, first, last, n);
    const auto size = cast<std::size_t>(last - first);
    while (n--) {
      std::memcpy(out, first, sizeof(T) * size);
      out += size;
    }
    return out;
  }

  template <typename Char, ranges::output_iterator<const Char&> Out,
            class Size = std::size_t>
  constexpr Out copy_to(Out out, std::basic_string_view<Char> sv, Size n = 1) {
    return copy_to(out, sv.data(), sv.data() + sv.size(), n);
  }

  // style_format_to

  template <typename Char, ranges::output_iterator<const Char&> Out>
  auto style_format_to(Out out, const fmt::text_style& style)
    -> std::pair<Out, bool> {
    bool has_style = false;
    if (style.has_emphasis()) {
      has_style = true;
      auto emphasis = fmt::detail::make_emphasis<Char>(style.get_emphasis());
      out = fmt::detail::write(out, (const Char*)emphasis);
    }
    if (style.has_foreground()) {
      has_style = true;
      auto foreground =
        fmt::detail::make_foreground_color<Char>(style.get_foreground());
      out = fmt::detail::write(out, (const Char*)foreground);
    }
    if (style.has_background()) {
      has_style = true;
      auto background =
        fmt::detail::make_background_color<Char>(style.get_background());
      out = fmt::detail::write(out, (const Char*)background);
    }
    return {out, has_style};
  }

  // reset_style

  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out reset_style(Out out) {
    return fmt::detail::write(out, RICH_TYPED_LITERAL(Char, "\x1b[0m"));
  }

  // format_to

  // https://github.com/fmtlib/fmt/blob/fd41110d383b7240231718f009b21498e3984ccc/include/fmt/format.h#L1645-L1661
  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out padded_format_to(Out out, const fmt::text_style& style,
                       std::basic_string_view<Char> sv,
                       std::basic_string_view<Char> fill,
                       const std::size_t left, const std::size_t right) {
    if (sv.empty() and (fill.empty() or (left == 0 and right == 0)))
      return out;
    bool has_style;
    std::tie(out, has_style) = style_format_to<Char>(out, style);
    if (not fill.empty() and left > 0)
      out = copy_to<Char>(out, fill, left);
    if (not sv.empty())
      out = copy_to<Char>(out, sv);
    if (not fill.empty() and right > 0)
      out = copy_to<Char>(out, fill, right);
    if (has_style)
      out = reset_style<Char>(out);
    return out;
  }

  enum class align_t : std::uint8_t {
    left,
    center,
    right,
  };

  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out aligned_format_to(Out out, const fmt::text_style& style,
                        std::basic_string_view<Char> sv,
                        std::basic_string_view<Char> fill, const align_t align,
                        const std::size_t width) {
    switch (align) {
    case align_t::left:
      return padded_format_to(out, style, sv, fill, 0, width);
    case align_t::center: {
      const auto left = width / 2;
      return padded_format_to(out, style, sv, fill, left, width - left);
    }
    case align_t::right:
      return padded_format_to(out, style, sv, fill, width, 0);
    default:
      RICH_UNREACHABLE();
    }
  }

  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out reversed_format_to(Out out, const fmt::text_style& style,
                         std::basic_string_view<Char> sv,
                         std::basic_string_view<Char> fill, const align_t align,
                         const std::size_t width) {
    switch (align) {
    case align_t::left:
      return padded_format_to(out, style, sv, fill, width, 0);
    case align_t::center: {
      const auto right = width / 2;
      return padded_format_to(out, style, sv, fill, width - right, right);
    }
    case align_t::right:
      return padded_format_to(out, style, sv, fill, 0, width);
    default:
      RICH_UNREACHABLE();
    }
  }
} // namespace rich
