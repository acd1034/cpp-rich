/// @file math.hpp
#pragma once
#include <cmath> // std::log10, std::llround

#include <rich/fundamental.hpp>

namespace rich {

  /// ilog10
  template <std::integral T>
  T ilog10(T t) {
    return icast<T>(std::llround(std::log10(static_cast<double>(t))));
  }

  // saturation arithmetic
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0543r1.html
  // https://locklessinc.com/articles/sat_arithmetic/

  std::size_t sat_add(std::size_t x, std::size_t y) noexcept {
    std::size_t ret = x + y;
    ret |= -(ret < x);
    return ret;
  }

  std::size_t sat_sub(std::size_t x, std::size_t y) noexcept {
    std::size_t ret = x - y;
    ret &= -(ret <= x);
    return ret;
  }
} // namespace rich
