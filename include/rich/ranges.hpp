/// @file ranges.hpp
#pragma once
#include <ranges>

#include <rich/fundamental.hpp>

namespace rich::ranges::detail {

  // range operations
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/range/operations.hpp

  // clang-format off

  struct index_fn {
    /// @return `begin(rng)[n]`
    template <typename Rng, typename Int>
    requires std::ranges::random_access_range<Rng>
      and std::integral<Int> and std::ranges::borrowed_range<Rng>
    constexpr std::ranges::range_reference_t<Rng>
    operator()(Rng&& rng, Int n) const {
      using D = std::ranges::range_difference_t<Rng>;
      assert(0 <= static_cast<D>(n));
      assert(not std::ranges::sized_range<Rng>
               or static_cast<D>(n) < std::ranges::distance(rng));
      return std::ranges::begin(rng)[static_cast<D>(n)];
    }
  };

  template <typename T>
  concept can_invoke_empty = requires(T&& t) { std::ranges::empty(t); };

  struct front_fn {
    /// @return `*begin(rng)`
    template <typename Rng>
    requires std::ranges::forward_range<Rng>
      and std::ranges::borrowed_range<Rng>
    constexpr std::ranges::range_reference_t<Rng> operator()(Rng&& rng) const {
      assert(not can_invoke_empty<Rng> or not std::ranges::empty(rng));
      return *std::ranges::begin(rng);
    }
  };

  struct back_fn {
    /// @return `*prev(end(rng))`
    template <typename Rng>
    requires std::ranges::common_range<Rng>
      and std::ranges::bidirectional_range<Rng>
      and std::ranges::borrowed_range<Rng>
    constexpr std::ranges::range_reference_t<Rng> operator()(Rng&& rng) const {
      assert(not can_invoke_empty<Rng> or not std::ranges::empty(rng));
      return *std::ranges::prev(std::ranges::end(rng));
    }
  };

  // clang-format on
} // namespace rich::ranges::detail

namespace rich::ranges::inline cpo {
  inline constexpr index_fn index{};
  inline constexpr front_fn front{};
  inline constexpr back_fn back{};
} // namespace rich::ranges::inline cpo
