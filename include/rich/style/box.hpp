/// @file box.hpp
#pragma once
#include <array>
#include <string_view>

#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  using box_t = std::span<const std::basic_string_view<Char>>;
}

namespace rich::box {
// S1, S4, and S7 must be exactly one character.
// Others must be zero or one character.
#define RICH_DEFINE_BOX(Name, S0, S1, S2, S3, S4, S5, S6, S7, S8)              \
  namespace detail {                                                           \
    template <typename Char>                                                   \
    inline constexpr std::array Name##Instance{                                \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S0)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S1)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S2)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S3)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S4)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S5)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S6)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S7)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S8)),              \
    };                                                                         \
  }                                                                            \
  template <typename Char>                                                     \
  inline constexpr auto Name = std::span(detail::Name##Instance<Char>)

  // ╭─╮
  // │ │
  // ╰─╯
  RICH_DEFINE_BOX(Rounded, "╭", "─", "╮", "│", " ", "│", "╰", "─", "╯");

  // ┌─┐
  // │ │
  // └─┘
  RICH_DEFINE_BOX(Squared, "┌", "─", "┐", "│", " ", "│", "└", "─", "┘");

  // ▛▀▜
  // ▌ ▐
  // ▙▄▟
  RICH_DEFINE_BOX(HalfBlock, "▛", "▀", "▜", "▌", " ", "▐", "▙", "▄", "▟");

  // _ _
  // _ _
  // _ _
  RICH_DEFINE_BOX(NoBorder, "", " ", "", "", " ", "", "", " ", "");

  // ● ↵
  // _ ↵
  // _ ↵
  RICH_DEFINE_BOX(Editor, "●", " ", "↵", "", " ", "↵", "", " ", "↵");

#undef RICH_DEFINE_BOX

#define RICH_DEFINE_BOX(Name, S0, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10,     \
                        S11, S12, S13, S14, S15)                               \
  namespace detail {                                                           \
    template <typename Char>                                                   \
    inline constexpr std::array Name##Instance{                                \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S0)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S1)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S2)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S3)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S4)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S5)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S6)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S7)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S8)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S9)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S10)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S11)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S12)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S13)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S14)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S15)),             \
    };                                                                         \
  }                                                                            \
  template <typename Char>                                                     \
  inline constexpr auto Name = std::span(detail::Name##Instance<Char>)

  // ╭─┬╮
  // │ ││
  // ├─┼┤
  // ╰─┴╯
  RICH_DEFINE_BOX(Rounded2, "╭", "─", "┬", "╮", "│", " ", "│", "│", "├", "─", "┼", "┤", "╰", "─", "┴", "╯");
} // namespace rich::box
