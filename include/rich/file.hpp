/// @file file.hpp
#pragma once
#include <fstream>
#include <string>
#include <string_view>

#include <rich/exception.hpp>

namespace rich {
  // https://kagasu.hatenablog.com/entry/2017/05/01/215219
  std::string get_file_contents(const char* fname) {
    std::ifstream ifs(fname);
    if (!ifs)
      throw runtime_error("Failed to read file");
    std::string ret((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());
    return ret;
  }

  std::string get_file_contents(const std::string& fname) {
    return get_file_contents(fname.c_str());
  }

  template <class Char, class Traits>
  constexpr std::size_t find_nth(std::basic_string_view<Char, Traits> sv,
                                 const Char c, std::size_t n,
                                 std::size_t pos = 0) noexcept {
    if (n == 0)
      return pos;
    while (--n > 0) {
      pos = sv.find(c, pos);
      if (pos == std::basic_string_view<Char, Traits>::npos)
        return pos;
      ++pos;
    }
    return sv.find(c, pos);
  }

  std::string_view extract_partial_contents(std::string_view contents,
                                            const std::uint_least32_t line,
                                            const std::size_t extra_line) {
    auto l = cast<std::int32_t>(line) - 1;
    auto e = cast<std::int32_t>(extra_line);
    auto a = std::max(l - e, cast<std::int32_t>(0));
    auto first = find_nth(contents, '\n', cast<std::size_t>(a));
    if (first != std::string_view::npos)
      ++first;
    auto last = find_nth(contents, '\n', cast<std::size_t>(e) * 2, first);
    return contents.substr(first, last - first);
  }
} // namespace rich
