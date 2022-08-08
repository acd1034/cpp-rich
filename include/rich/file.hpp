/// @file file.hpp
#pragma once
#include <fstream>
#include <string>
#include <string_view>
#include <range/v3/view.hpp>
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

  namespace _ranges = ::ranges;
  namespace _views = ::ranges::views;

  std::string extract_partial_contents(std::string_view contents,
                                       std::uint_least32_t line,
                                       std::uint_least32_t num) {
    auto l = std::int_least32_t(line) - 1;
    auto n = std::int_least32_t(num);
    auto a = std::max(l - n / 2, std::int_least32_t(0));
    auto extracted = contents | _views::split('\n') | _views::drop(a)
                     | _views::take(n) | _views::join('\n') | _views::common;
    std::string ret(_ranges::begin(extracted), _ranges::end(extracted));
    return ret;
  }
} // namespace rich
