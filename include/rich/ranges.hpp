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

  // accumulate
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/numeric/accumulate.hpp#

  struct accumulate_fn {
    template<typename I, typename S, typename T, typename Op = std::plus<>,
             typename P = std::identity>
    requires std::sentinel_for<S, I> and std::input_iterator<I>
      and std::indirectly_binary_invocable<Op, T*, std::projected<I, P>>
      and std::assignable_from<T&, std::indirect_result_t<Op&, T*, std::projected<I, P>>>
    constexpr T
    operator()(I first, S last, T init, Op op = Op{}, P proj = P{}) const {
      for (; first != last; ++first)
        init = std::invoke(op, init, std::invoke(proj, *first));
      return init;
    }

    template<typename Rng, typename T, typename Op = std::plus<>,
             typename P = std::identity>
    requires std::ranges::input_range<Rng>
      and std::indirectly_binary_invocable_<Op, T*, std::projected<std::ranges::iterator_t<Rng>, P>>
      and std::assignable_from<T&, std::indirect_result_t<Op&, T*, std::projected<std::ranges::iterator_t<Rng>, P>>>
    constexpr T
    operator()(Rng&& rng, T init, Op op = Op{}, P proj = P{}) const {
      return (*this)(std::ranges::begin(rng), std::ranges::end(rng),
                     std::move(init), std::move(op), std::move(proj));
    }
  };

  // clang-format on
} // namespace rich::ranges::detail

namespace rich::ranges::inline cpo {
  inline constexpr index_fn index{};
  inline constexpr front_fn front{};
  inline constexpr back_fn back{};
  inline constexpr accumulate_fn accumulate{};
} // namespace rich::ranges::inline cpo
