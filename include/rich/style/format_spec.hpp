/// @file format_spec.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/saturation.hpp>

namespace rich {
  template <typename Char>
  struct format_spec {
    fmt::text_style style{};
    std::basic_string_view<Char> fill{};
    align_t align{};
    std::size_t width{};
  };

  // `sv` must be zero or one character.
  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out spec_format_to(Out out, const format_spec<Char>& fs,
                     std::basic_string_view<Char> sv) {
    auto width = sv.empty() ? fs.width : sat_sub(fs.width, 1);
    return aligned_format_to(out, fs.style, sv, fs.fill, fs.align, width);
  }

  // `sv` must be zero or one character.
  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out rspec_format_to(Out out, const format_spec<Char>& fs,
                      std::basic_string_view<Char> sv) {
    auto width = sv.empty() ? fs.width : sat_sub(fs.width, 1);
    return reversed_format_to(out, fs.style, sv, fs.fill, fs.align, width);
  }
} // namespace rich
