/// @file format_spec.hpp
#pragma once
#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  struct format_spec {
    fmt::text_style style{};
    std::basic_string_view<Char> fill{};
    align_t align{};
    std::size_t width{};
  };

  template <typename Char>
  auto invert(format_spec<Char> fs) {
    switch (fs.align) {
    case align_t::left:
      fs.align = align_t::right;
      break;
    case align_t::right:
      fs.align = align_t::left;
      break;
    }
    return fs;
  }

  template <typename Char, ranges::output_iterator<const Char&> Out>
  Out spec_format_to(Out out, const format_spec<Char>& fs,
                     std::basic_string_view<Char> sv) {
    return aligned_format_to(out, fs.style, sv, fs.fill, fs.align, fs.width);
  }
} // namespace rich
