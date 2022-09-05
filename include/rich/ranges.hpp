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

  // copy
  // https://github.com/llvm/llvm-project/blob/30a4264f5fb24ee34de5a61cc8ba8d9d879b5af5/libcxx/include/__algorithm/copy.h#L30-L58
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/algorithm/copy.hpp

  struct copy_fn {
  private:
    template <class I, class S, class O>
    constexpr std::pair<I, O>
    copy_impl(I first, S last, O out) const {
      for(; first != last; ++first, ++out)
        *out = *first;
      return {first, out};
    }

    template <class T, class U>
    requires std::same_as<std::remove_const_t<T>, U> and
      std::is_trivially_copy_assignable_v<U>
    constexpr std::pair<T*, U*>
    copy_impl(T* first, T* last, U* out) const {
      if (std::is_constant_evaluated())
        return copy_impl<T*, T*, U*>(first, last, out);
      const auto n = icast<std::size_t>(last - first);
      if (n > 0)
        std::memmove(out, first, n * sizeof(U));
      return {first + n, out + n};
    }

  public:
    template <class I, class S, class O>
    requires std::input_iterator<I> and std::sentinel_for<S, I> and
      std::weakly_incrementable<O> and std::indirectly_copyable<I, O>
    constexpr std::pair<I, O>
    operator()(I first, S last, O out) const {
      return copy_impl(std::move(first), std::move(last), std::move(out));
    }

    template <class R, class O>
    requires std::ranges::input_range<R> and std::weakly_incrementable<O> and
      std::indirectly_copyable<std::ranges::iterator_t<R>, O>
    constexpr std::pair<std::ranges::borrowed_iterator_t<R>, O>
    operator()(R&& r, O out) const {
      return copy_impl(std::ranges::begin(r), std::ranges::end(r),
                       std::move(out));
    }
  };

  // indirectly_binary_invocable
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/iterator/concepts.hpp#L569-L593

    template <class Op, class I1, class I2>
    concept indirectly_binary_invocable =
      std::indirectly_readable<I1> and std::indirectly_readable<I2>
      and std::copy_constructible<Op>
      and std::invocable<Op&, std::iter_reference_t<I1>, std::iter_reference_t<I2>>;

  // accumulate
  // https://github.com/ericniebler/range-v3/blob/234164b84797f2a6ec97fdfb4d1c5dbfb927ca35/include/range/v3/numeric/accumulate.hpp

  struct accumulate_fn {
    template <class I, class S, class T, class Op = std::plus<>,
              class P = std::identity>
    requires std::sentinel_for<S, I> and std::input_iterator<I>
      and indirectly_binary_invocable<Op, T*, std::projected<I, P>>
      and std::assignable_from<T&, std::indirect_result_t<Op&, T*, std::projected<I, P>>>
    constexpr T
    operator()(I first, S last, T init, Op op = Op{}, P proj = P{}) const {
      for (; first != last; ++first)
        init = std::invoke(op, std::move(init), std::invoke(proj, *first));
      return init;
    }

    template <class R, class T, class Op = std::plus<>,
              class P = std::identity>
    requires std::ranges::input_range<R>
      and indirectly_binary_invocable<Op, T*, std::projected<std::ranges::iterator_t<R>, P>>
      and std::assignable_from<T&, std::indirect_result_t<Op&, T*, std::projected<std::ranges::iterator_t<R>, P>>>
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
  inline constexpr detail::copy_fn copy{};
  inline constexpr detail::accumulate_fn accumulate{};
} // namespace rich::ranges::inline cpo
