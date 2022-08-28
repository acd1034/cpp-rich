/// @file fundamental.hpp
#pragma once
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef> // std::size_t, std::ptrdiff_t, std::nullptr_t
#include <cstdint> // std::int32_t
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility> // std::move, std::forward, std::swap, std::exchange

// TODO: granularize
#include <iterator>
#include <ranges>

namespace rich {
// RICH_UNREACHABLE
#define RICH_UNREACHABLE() (assert(false))

  /// always_false
  template <class>
  inline constexpr bool always_false = false;

  /// contextually_convertible_to_bool
  template <class B>
  concept contextually_convertible_to_bool = requires(B&& b) {
    bool(std::forward<B>(b));
  };

  /// cast
  template <std::integral To, std::integral From>
  constexpr To cast(From from) noexcept(noexcept(static_cast<To>(from))) {
    assert(std::in_range<To>(from));
    return static_cast<To>(from);
  }

  /// to_unsigned
  template <std::integral From>
  constexpr std::make_unsigned_t<From> to_unsigned(From from) noexcept(
    noexcept(cast<std::make_unsigned_t<From>>(from))) {
    return cast<std::make_unsigned_t<From>>(from);
  }

  /// to_signed
  template <std::integral From>
  constexpr std::make_signed_t<From> to_signed(From from) noexcept(
    noexcept(cast<std::make_signed_t<From>>(from))) {
    return cast<std::make_signed_t<From>>(from);
  }
} // namespace rich
