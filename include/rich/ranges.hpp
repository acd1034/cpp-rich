/// @file ranges.hpp
#pragma once
#include <functional> // std::invoke

#include <rich/fundamental.hpp>

namespace rich::ranges::detail {

  // clang-format off

  // range operations
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/range/operations.hpp

  struct index_fn {
    /// @return `begin(r)[n]`
    template <class R, class Int>
    requires std::ranges::random_access_range<R>
      and std::integral<Int> and std::ranges::borrowed_range<R>
    constexpr std::ranges::range_reference_t<R>
    operator()(R&& r, Int n) const {
      using D = std::ranges::range_difference_t<R>;
      assert(0 <= static_cast<D>(n));
      assert(not std::ranges::sized_range<R>
               or static_cast<D>(n) < std::ranges::distance(r));
      return std::ranges::begin(r)[static_cast<D>(n)];
    }
  };

  template <class T>
  concept can_invoke_empty = requires(T&& t) { std::ranges::empty(t); };

  struct front_fn {
    /// @return `*begin(r)`
    template <class R>
    requires std::ranges::forward_range<R>
      and std::ranges::borrowed_range<R>
    constexpr std::ranges::range_reference_t<R> operator()(R&& r) const {
      assert(not can_invoke_empty<R> or not std::ranges::empty(r));
      return *std::ranges::begin(r);
    }
  };

  struct back_fn {
    /// @return `*prev(end(r))`
    template <class R>
    requires std::ranges::common_range<R>
      and std::ranges::bidirectional_range<R>
      and std::ranges::borrowed_range<R>
    constexpr std::ranges::range_reference_t<R> operator()(R&& r) const {
      assert(not can_invoke_empty<R> or not std::ranges::empty(r));
      return *std::ranges::prev(std::ranges::end(r));
    }
  };

  // accumulate
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/iterator/concepts.hpp#L569-L593
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/numeric/accumulate.hpp

  template <class I, class T, class Op = std::plus<>, class P = std::identity>
  concept accumulate_constraint =
    std::invocable<P&, std::iter_reference_t<I>>
    and std::invocable<Op&, T, std::invoke_result_t<P&, std::iter_reference_t<I>>>
    and std::assignable_from<T&, std::invoke_result_t<Op&, T, std::invoke_result_t<P&, std::iter_reference_t<I>>>>;

  struct accumulate_fn {
    template <class I, class S, class T, class Op = std::plus<>,
              class P = std::identity>
    requires std::sentinel_for<S, I> and std::input_iterator<I>
      and accumulate_constraint<I, T, Op, P>
    constexpr T
    operator()(I first, S last, T init, Op op = Op{}, P proj = P{}) const {
      for (; first != last; ++first)
        init = std::invoke(op, init, std::invoke(proj, *first));
      return init;
    }

    template <class R, class T, class Op = std::plus<>,
              class P = std::identity>
    requires std::ranges::input_range<R>
      and accumulate_constraint<std::ranges::iterator_t<R>, T, Op, P>
    constexpr T
    operator()(R&& r, T init, Op op = Op{}, P proj = P{}) const {
      return (*this)(std::ranges::begin(r), std::ranges::end(r),
                     std::move(init), std::move(op), std::move(proj));
    }
  };

  // clang-format on
} // namespace rich::ranges::detail

namespace rich::ranges::inline cpo {
  inline constexpr detail::index_fn index{};
  inline constexpr detail::front_fn front{};
  inline constexpr detail::back_fn back{};
  inline constexpr detail::accumulate_fn accumulate{};
} // namespace rich::ranges::inline cpo
