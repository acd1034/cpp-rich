/// @file saturation.hpp
#pragma once
#include <rich/fundamental.hpp>

namespace rich {
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
