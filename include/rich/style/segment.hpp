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
    bool has_style = false;
    auto out = ctx.out();
    if (seg.style().has_emphasis()) {
      has_style = true;
      auto emphasis =
        fmt::detail::make_emphasis<Char>(seg.style().get_emphasis());
      out = fmt::detail::write(out, (const Char*)emphasis);
    }
    if (seg.style().has_foreground()) {
      has_style = true;
      auto foreground =
        fmt::detail::make_foreground_color<Char>(seg.style().get_foreground());
      out = fmt::detail::write(out, (const Char*)foreground);
    }
    if (seg.style().has_background()) {
      has_style = true;
      auto background =
        fmt::detail::make_background_color<Char>(seg.style().get_background());
      out = fmt::detail::write(out, (const Char*)background);
    }
    out = fmtr.format(seg.text(), ctx);
    if (has_style)
      out = fmt::detail::write(out, RICH_TYPED_LITERAL(Char, "\x1b[0m"));
    return out;
  }
};
