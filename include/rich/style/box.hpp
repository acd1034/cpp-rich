/// @file box.hpp
#pragma once
#include <array>
#include <string_view>

#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  using box_t = std::span<const std::basic_string_view<Char>>;

  // clang-format off

  // ┌─── left
  // │┌── mid
  // ││┌─ col
  // │││┌ right
  // ↓↓↓↓
  // ┌─┬┐ top
  // │ ││ mid
  // ├─┼┤ row
  // └─┴┘ bottom
  template <typename Char> auto top_left (box_t<Char> b) { return b[0]; }
  template <typename Char> auto top_mid  (box_t<Char> b) { return b[1]; }
  template <typename Char> auto top_col  (box_t<Char> b) { return b[2]; }
  template <typename Char> auto top_right(box_t<Char> b) { return b[3]; }
  template <typename Char> auto mid_left (box_t<Char> b) { return b[4]; }
  template <typename Char> auto mid_mid  (box_t<Char> b) { return b[5]; }
  template <typename Char> auto mid_col  (box_t<Char> b) { return b[6]; }
  template <typename Char> auto mid_right(box_t<Char> b) { return b[7]; }
  template <typename Char> auto row_left (box_t<Char> b) { return b[8]; }
  template <typename Char> auto row_mid  (box_t<Char> b) { return b[9]; }
  template <typename Char> auto row_col  (box_t<Char> b) { return b[10]; }
  template <typename Char> auto row_right(box_t<Char> b) { return b[11]; }
  template <typename Char> auto bottom_left (box_t<Char> b) { return b[12]; }
  template <typename Char> auto bottom_mid  (box_t<Char> b) { return b[13]; }
  template <typename Char> auto bottom_col  (box_t<Char> b) { return b[14]; }
  template <typename Char> auto bottom_right(box_t<Char> b) { return b[15]; }

  // clang-format on
} // namespace rich

namespace rich::box {
// S1, S5, S9 and S13 must be exactly one character.
// Others must be zero or one character.
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

  // clang-format off

  // ╭─┬╮
  // │ ││
  // ├─┼┤
  // ╰─┴╯
  RICH_DEFINE_BOX(Rounded, "╭", "─", "┬", "╮",
                           "│", " ", "│", "│",
                           "├", "─", "┼", "┤",
                           "╰", "─", "┴", "╯");

  // ╭──╮
  // │  │
  // │  │
  // ╰──╯
  RICH_DEFINE_BOX(RoundedNoSep, "╭", "─", "─", "╮",
                                "│", " ", " ", "│",
                                "│", " ", " ", "│",
                                "╰", "─", "─", "╯");

  // ┌─┬┐
  // │ ││
  // ├─┼┤
  // └─┴┘
  RICH_DEFINE_BOX(Square, "┌", "─", "┬", "┐",
                          "│", " ", "│", "│",
                          "├", "─", "┼", "┤",
                          "└", "─", "┴", "┘");

  // ╭──╮
  // │  │
  // │  │
  // ╰──╯
  RICH_DEFINE_BOX(SquareNoSep, "┌", "─", "─", "┐",
                               "│", " ", " ", "│",
                               "│", " ", " ", "│",
                               "└", "─", "─", "┘");

  // _ __
  // _ __
  // _ __
  // _ __
  RICH_DEFINE_BOX(NoBorder, "", " ", "", "",
                            "", " ", "", "",
                            "", " ", "", "",
                            "", " ", "", "");

  // ● _↵
  // _ _↵
  // _ _↵
  // _ _↵
  RICH_DEFINE_BOX(Editor, "●", " ", "", "↵",
                          "", " ", "", "↵",
                          "", " ", "", "↵",
                          "", " ", "", "↵");

  // clang-format on
#undef RICH_DEFINE_BOX
} // namespace rich::box
