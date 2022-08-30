/// @file segment.hpp
#pragma once
#include <string_view>

#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  struct segment {
  private:
    using string_view_type = std::basic_string_view<Char>;
    string_view_type text_{};
    fmt::text_style style_{};

  public:
    using char_type = Char;

    // ctor
    segment() = default;
    constexpr explicit segment(string_view_type t) : text_(t) {}
    constexpr segment(string_view_type t, const fmt::text_style& s)
      : text_(t), style_(s) {}

    // observer
    auto& text() { return text_; }
    auto text() const { return text_; }
    auto& style() { return style_; }
    const auto& style() const { return style_; }
  };

  template <class T>
  struct is_segment : std::false_type {};

  template <typename Char>
  struct is_segment<segment<Char>> : std::true_type {};

  template <class T>
  inline constexpr bool is_segment_v = is_segment<T>::value;
} // namespace rich

template <typename Char>
struct fmt::formatter<rich::segment<Char>, Char> {
private:
  fmt::formatter<std::basic_string_view<Char>, Char> fmtr{};

public:
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return fmtr.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const rich::segment<Char>& seg, FormatContext& ctx) const
    -> decltype(ctx.out()) {
    auto out = ctx.out();
    bool has_style;
    std::tie(out, has_style) = rich::style_format_to<Char>(out, seg.style());
    ctx.advance_to(out);
    out = fmtr.format(seg.text(), ctx);
    if (has_style)
      out = rich::reset_style<Char>(out);
    return out;
  }
};
