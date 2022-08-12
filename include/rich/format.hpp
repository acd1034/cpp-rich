/// @file format.hpp
#pragma once
#include <fmt/color.h>
#include <fmt/ranges.h>

#include <rich/fundamental.hpp>

namespace rich {

  // RICH_TYPED_LITERAL
  // See https://github.com/microsoft/STL/blob/17fde2cbab6e8724d81c9555237c9a623d7fb954/tests/std/tests/P0220R1_string_view/test.cpp#L260-L277

  template <class CharT>
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

#define RICH_TYPED_LITERAL(CharT, Literal)                                          \
  (rich::choose_literal<CharT>::choose(Literal, L##Literal))
} // namespace rich
