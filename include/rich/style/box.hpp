/// @file box.hpp
#pragma once
#include <array>
#include <string_view>

#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  using box_type = std::array<std::basic_string_view<Char>, 9>;

// S1, S4, and S7 must be exactly one character.
// Others must be zero or one character.
#define RICH_DEFINE_BOX(Name, S0, S1, S2, S3, S4, S5, S6, S7, S8)              \
  template <typename Char>                                                     \
  inline constexpr std::array Name {                                           \
    std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S0)),                \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S1)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S2)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S3)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S4)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S5)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S6)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S7)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S8)),              \
  }

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

  // ● ↵
  // _ ↵
  // _ ↵
  RICH_DEFINE_BOX(Editor, "●", " ", "↵", "", " ", "↵", "", " ", "↵");

#undef RICH_DEFINE_BOX
} // namespace rich
