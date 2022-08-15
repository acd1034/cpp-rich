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
#include <range/v3/view.hpp>

namespace rich {
  namespace _ranges = ::ranges;
  namespace _views = ::ranges::views;

  /// squared
  template <class X>
  constexpr auto squared(const X& x) noexcept(noexcept(x* x))
    -> decltype(x * x) {
    return x * x;
  }

// RICH_UNREACHABLE
#define RICH_UNREACHABLE() (assert(false))

  /// always_false
  template <class>
  inline constexpr bool always_false = false;

  template <class B>
  concept boolean_testable_impl = std::convertible_to<B, bool>;

  /// boolean_testable
  template <class B>
  concept boolean_testable = boolean_testable_impl<B> and requires(B&& b) {
    { !std::forward<B>(b) } -> boolean_testable_impl;
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
