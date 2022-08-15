/// @file format_spec.hpp
#pragma once
#include <rich/format.hpp>

namespace rich {
  template <typename Char>
  struct format_spec {
    std::basic_string_view<Char> fill{};
    Char align{};
    std::size_t width{};
    fmt::text_style style{};
  };

  template <typename Char>
  auto invert(format_spec<Char>);

  template <>
  auto invert(format_spec<char> fs) {
    switch (fs.align) {
    case '<':
      fs.align = '>';
      break;
    case '>':
      fs.align = '<';
      break;
    }
    return fs;
  }

  template <class Out, typename Char>
  requires ranges::output_iterator<Out, const Char&>
    Out _format_to(Out out, const format_spec<Char>& fs,
                   std::basic_string_view<Char> sv) {
    const auto fmtstr =
      fmt::format("{{:{0}{1}{2}}}", fs.fill, fs.align, fs.width);
    return fmt::vformat_to(out, fs.style, fmt::detail::to_string_view(fmtstr),
                           fmt::make_format_args(sv));
  }

  template <class Out, typename Char>
  requires ranges::output_iterator<Out, const Char&>
    Out _format_to(Out out, const format_spec<Char>& fs, const Char* s) {
    return _format_to(out, fs, std::basic_string_view<Char>(s));
  }
} // namespace rich
