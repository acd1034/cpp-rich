#ifndef RICH_SINGLE_HEADER_INCLUDED
#define RICH_SINGLE_HEADER_INCLUDED
#include <exception>
#include <source_location>
#include <string>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef> // std::size_t, std::ptrdiff_t, std::nullptr_t
#include <cstdint> // std::int32_t
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility> // std::move, std::forward, std::swap, std::exchange
#include <iterator>
#include <ranges>
#include <fstream>
#include <string_view>
#include <cmath> // std::log10, std::llround
#include <fmt/color.h>
#include <functional> // std::invoke
#include <memory> // std::shared_ptr
#include <optional>
#include <regex>
#include <span>
#include <array>
#include <any>
#include <vector>
#include <list>

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/rich.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/exception.hpp
/// @file exception.hpp

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/fundamental.hpp
/// @file fundamental.hpp

// TODO: granularize

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

  /// icast
  template <std::integral To, std::integral From>
  constexpr To icast(From from) noexcept(noexcept(static_cast<To>(from))) {
    assert(std::in_range<To>(from));
    return static_cast<To>(from);
  }

  /// make_reserved
  template <class Cont>
  // clang-format off
  requires(not std::is_reference_v<Cont> and requires(Cont & c, std::size_t n) {
    c.reserve(n);
  })
  // clang-format on
  Cont make_reserved(const std::size_t n) {
    Cont c;
    c.reserve(n);
    return c;
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/fundamental.hpp

namespace rich {
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/exception
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/stdexcept
  // https://github.com/llvm/llvm-project/tree/main/libcxx/src/support/runtime
  // https://stackoverflow.com/questions/28640553/exception-class-with-a-char-constructor
  struct exception : std::exception {
  private:
    std::string msg_{};
    std::source_location loc_{};

  public:
    explicit exception(const std::string& msg,
                       std::source_location loc = std::source_location::current())
      : msg_(msg), loc_(loc) {}
    explicit exception(const char* msg,
                       std::source_location loc = std::source_location::current())
      : msg_(msg), loc_(loc) {}

    exception(const exception&) noexcept = default;
    exception& operator=(const exception&) noexcept = default;

    ~exception() noexcept override = default;
    const char* what() const noexcept override { return msg_.c_str(); }
    std::source_location where() const noexcept { return loc_; }
  };

  // 雑に使える例外
  struct runtime_error : exception {
    using exception::exception;
  };
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/exception.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/file.hpp
/// @file file.hpp

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/math.hpp
/// @file math.hpp


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
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/math.hpp

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
    const auto l = icast<std::size_t>(line);
    const auto a = sat_sub(l, extra_line + 1);
    auto first = find_nth(contents, '\n', a);
    if (first != 0 and first != std::string_view::npos)
      ++first;
    auto last = find_nth(contents, '\n', extra_line * 2 + 1, first);
    return contents.substr(first, last - first);
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/file.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/format.hpp
/// @file format.hpp

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/ranges.hpp
/// @file ranges.hpp


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
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/ranges.hpp

namespace rich {

  // RICH_TYPED_LITERAL
  // See https://github.com/microsoft/STL/blob/17fde2cbab6e8724d81c9555237c9a623d7fb954/tests/std/tests/P0220R1_string_view/test.cpp#L260-L277

  template <class Char>
  struct choose_literal; // not defined

  template <>
  struct choose_literal<char> {
    static constexpr const char* choose(const char* s, const wchar_t*) {
      return s;
    }
  };

  template <>
  struct choose_literal<wchar_t> {
    static constexpr const wchar_t* choose(const char*, const wchar_t* s) {
      return s;
    }
  };

#define RICH_TYPED_LITERAL(Char, Literal)                                      \
  (rich::choose_literal<Char>::choose(Literal, L##Literal))

  // copy_to

  template <typename Char, std::output_iterator<const Char&> Out>
  constexpr Out copy_to(Out out, std::basic_string_view<Char> sv,
                        std::size_t n = 1) {
    while (n--)
      out = rich::ranges::copy(sv.data(), sv.data() + sv.size(), out).second;
    return out;
  }

  // set_style

  template <typename Char, std::output_iterator<const Char&> Out>
  auto set_style(Out out, const fmt::text_style& style)
    -> std::pair<Out, bool> {
    bool has_style = false;
    if (style.has_emphasis()) {
      has_style = true;
      auto emphasis = fmt::detail::make_emphasis<Char>(style.get_emphasis());
      out = copy_to<Char>(out, (const Char*)emphasis);
    }
    if (style.has_foreground()) {
      has_style = true;
      auto foreground =
        fmt::detail::make_foreground_color<Char>(style.get_foreground());
      out = copy_to<Char>(out, (const Char*)foreground);
    }
    if (style.has_background()) {
      has_style = true;
      auto background =
        fmt::detail::make_background_color<Char>(style.get_background());
      out = copy_to<Char>(out, (const Char*)background);
    }
    return {out, has_style};
  }

  // reset_style

  template <typename Char, std::output_iterator<const Char&> Out>
  Out reset_style(Out out) {
    return copy_to<Char>(out, RICH_TYPED_LITERAL(Char, "\x1b[0m"));
  }

  // format_to

  // https://github.com/fmtlib/fmt/blob/fd41110d383b7240231718f009b21498e3984ccc/include/fmt/format.h#L1645-L1661
  template <typename Char, std::output_iterator<const Char&> Out>
  Out padded_format_to(Out out, const fmt::text_style& style,
                       std::basic_string_view<Char> sv,
                       std::basic_string_view<Char> fill,
                       const std::size_t left, const std::size_t right) {
    if (sv.empty() and (fill.empty() or (left == 0 and right == 0)))
      return out;
    auto [out2, has_style] = set_style<Char>(out, style);
    out = out2;
    if (not fill.empty())
      out = copy_to<Char>(out, fill, left);
    if (not sv.empty())
      out = copy_to<Char>(out, sv);
    if (not fill.empty())
      out = copy_to<Char>(out, fill, right);
    if (has_style)
      out = reset_style<Char>(out);
    return out;
  }

  enum class align_t : std::uint8_t {
    left,
    center,
    right,
  };

  constexpr std::pair<std::size_t, std::size_t>
  padding_size(const align_t align, const std::size_t width) {
    switch (align) {
    case align_t::left:
      return {0, width};
    case align_t::center: {
      const auto left = width / 2;
      return {left, width - left};
    }
    case align_t::right:
      return {width, 0};
    default:
      RICH_UNREACHABLE();
    }
  }

  template <typename Char, std::output_iterator<const Char&> Out>
  Out aligned_format_to(Out out, const fmt::text_style& style,
                        std::basic_string_view<Char> sv,
                        std::basic_string_view<Char> fill, const align_t align,
                        const std::size_t width) {
    const auto [left, right] = padding_size(align, width);
    return padded_format_to(out, style, sv, fill, left, right);
  }

  template <typename Char, std::output_iterator<const Char&> Out>
  Out reversed_format_to(Out out, const fmt::text_style& style,
                         std::basic_string_view<Char> sv,
                         std::basic_string_view<Char> fill, const align_t align,
                         const std::size_t width) {
    const auto [left, right] = padding_size(align, width);
    return padded_format_to(out, style, sv, fill, right, left);
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/format.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/iterator.hpp
/// @file iterator.hpp


namespace rich {
  template <class T>
  struct erased_output {
  private:
    using U = std::remove_reference_t<T>;
    using base_ptr_t = std::shared_ptr<void>;
    base_ptr_t out_;
    using handler_t = void(base_ptr_t&, U&);
    handler_t* handler_ = nullptr;

  public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    template <std::output_iterator<T> Out>
    constexpr explicit erased_output(Out o)
      : out_(std::make_shared<Out>(std::move(o))),
        handler_([](base_ptr_t& out, U& t) {
          *(*std::static_pointer_cast<Out>(out))++ = std::move(t);
        }) {}

    constexpr erased_output&
    operator=(const U& t) requires std::copy_constructible<U> {
      U tmp{t};
      handler_(out_, tmp);
      return *this;
    }

    constexpr erased_output& operator=(U&& t) {
      handler_(out_, t);
      return *this;
    }

    constexpr erased_output& operator*() { return *this; }
    constexpr erased_output& operator++() { return *this; }
    constexpr erased_output operator++(int) { return *this; }

    template <class Out, class U>
    requires std::output_iterator<Out, U>
    friend constexpr Out out(const erased_output<U>&);
  };

  template <class Out, class U>
  requires std::output_iterator<Out, U>
  constexpr Out out(const erased_output<U>& x) {
    return *std::static_pointer_cast<Out>(x.out_);
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/iterator.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/regex.hpp
/// @file regex.hpp


namespace rich {

  // sub_match

  template <class BiIter, class ST = std::char_traits<
                            typename std::sub_match<BiIter>::value_type>>
  auto to_string_view(const std::sub_match<BiIter>& sm) {
    return std::basic_string_view<typename std::sub_match<BiIter>::value_type,
                                  ST>(sm.first, sm.second);
  }

  template <class BiIter, class ST = std::char_traits<
                            typename std::sub_match<BiIter>::value_type>>
  std::optional<std::size_t>
  match_find(std::span<const std::sub_match<BiIter>> mo,
             const std::size_t n = 0) {
    for (std::size_t i = n; i < mo.size(); ++i)
      if (mo[i].matched)
        return i;
    return std::nullopt;
  }

  template <class BiIter, class ST = std::char_traits<
                            typename std::sub_match<BiIter>::value_type>>
  auto match_group(std::span<const std::sub_match<BiIter>> mo,
                   const std::size_t n) {
    assert(n < mo.size());
    return to_string_view<BiIter, ST>(mo[n]);
  }

  // regex_search

  template <class ST, class Allocator, class Char, class Traits>
  bool regex_search(
    std::basic_string_view<Char, ST> sv,
    std::match_results<typename std::basic_string_view<Char, ST>::const_pointer,
                       Allocator>& m,
    const std::basic_regex<Char, Traits>& re,
    std::regex_constants::match_flag_type flags =
      std::regex_constants::match_default) {
    return std::regex_search(sv.data(), sv.data() + sv.size(), m, re, flags);
  }

  // regex_iterator

  template <class ST, class Char, class Traits>
  class regex_iterator {
  public:
    using regex_type = std::basic_regex<Char, Traits>;
    using string_view_type = std::basic_string_view<Char, ST>;

  private:
    using value_type_impl =
      std::match_results<typename string_view_type::const_pointer>;

  public:
    using value_type =
      std::pair<string_view_type,
                decltype(std::optional(std::span(value_type_impl())))>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

  private:
    string_view_type rng_{};
    const regex_type* pregex_ = nullptr;
    std::regex_constants::match_flag_type flags_ =
      std::regex_constants::match_default;
    value_type_impl match_{};
    bool is_prefix_ = false;

    void incremental_search();

  public:
    regex_iterator() = default;
    regex_iterator(string_view_type sv, const regex_type& re,
                   std::regex_constants::match_flag_type m =
                     std::regex_constants::match_default);
    regex_iterator(string_view_type sv, const regex_type&& re,
                   std::regex_constants::match_flag_type m =
                     std::regex_constants::match_default) = delete;

    bool operator==(const regex_iterator& x) const;
    bool operator!=(const regex_iterator& x) const { return !(*this == x); }

    reference operator*() const {
      if (is_prefix_) {
        if (match_.empty())
          return {rng_, std::nullopt};
        return {to_string_view(match_.prefix()), std::nullopt};
      }
      assert(not match_.empty());
      return {to_string_view(match_[0]), std::span(match_).subspan(1)};
    }

    regex_iterator& operator++();
    regex_iterator operator++(int) {
      regex_iterator t(*this);
      ++(*this);
      return t;
    }
  };

  template <class ST, class Char, class Traits>
  regex_iterator<ST, Char, Traits>::regex_iterator(
    string_view_type sv, const regex_type& re,
    std::regex_constants::match_flag_type m)
    : rng_(sv), pregex_(std::addressof(re)), flags_(m), is_prefix_(true) {
    regex_search(rng_, match_, *pregex_, flags_);
  }

  template <class ST, class Char, class Traits>
  bool
  regex_iterator<ST, Char, Traits>::operator==(const regex_iterator& x) const {
    if (match_.empty() and x.match_.empty() and not is_prefix_
        and not x.is_prefix_)
      return true;
    if (match_.empty() == x.match_.empty() and is_prefix_ == x.is_prefix_)
      return rng_ == x.rng_ and pregex_ == x.pregex_ and flags_ == x.flags_
             and match_[0] == x.match_[0];
    return false;
  }

  template <class ST, class Char, class Traits>
  void regex_iterator<ST, Char, Traits>::incremental_search() {
    assert(pregex_ != nullptr);
    // flags_ |= std::regex_constants::no_update_pos;
    rng_ = to_string_view(match_.suffix());
    auto rng = rng_;
    if (to_string_view(match_[0]).empty()) {
      if (rng.empty()) {
        match_ = value_type_impl();
        return;
      } else if (regex_search(rng, match_, *pregex_,
                              flags_ | std::regex_constants::match_not_null
                                | std::regex_constants::match_continuous))
        return;
      else
        rng = rng.substr(1);
    }
    flags_ |= std::regex_constants::match_prev_avail;
    if (not regex_search(rng, match_, *pregex_, flags_))
      match_ = value_type_impl();
  }

  template <class ST, class Char, class Traits>
  regex_iterator<ST, Char, Traits>&
  regex_iterator<ST, Char, Traits>::operator++() {
    if (is_prefix_) {
      is_prefix_ = false;
      return *this;
    }
    assert(not match_.empty());
    is_prefix_ = true;
    incremental_search();
    return *this;
  }

  // regex_range

  template <class ST, class Char, class Traits>
  auto regex_range(std::basic_string_view<Char, ST> sv,
                   const std::basic_regex<Char, Traits>& re,
                   std::regex_constants::match_flag_type flags =
                     std::regex_constants::match_default) {
    return std::ranges::subrange(regex_iterator(sv, re, flags),
                                 decltype(regex_iterator(sv, re, flags))());
  }

  template <class ST, class Char, class Traits>
  auto regex_range(std::basic_string_view<Char, ST> sv,
                   const std::basic_regex<Char, Traits>&& re,
                   std::regex_constants::match_flag_type flags =
                     std::regex_constants::match_default) = delete;
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/regex.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/box.hpp
/// @file box.hpp


namespace rich {
  template <typename Char>
  using box_t = std::span<const std::basic_string_view<Char>>;

  // clang-format off

  // ┌─── left
  // │┌── mid
  // ││┌─ col
  // │││┌ right
  // ↓↓↓↓
  // ┌─┬┐ top
  // │ ││ mid
  // ├─┼┤ row
  // └─┴┘ bottom
  template <typename Char> auto top_left (box_t<Char> b) { return b[0]; }
  template <typename Char> auto top_mid  (box_t<Char> b) { return b[1]; }
  template <typename Char> auto top_col  (box_t<Char> b) { return b[2]; }
  template <typename Char> auto top_right(box_t<Char> b) { return b[3]; }
  template <typename Char> auto mid_left (box_t<Char> b) { return b[4]; }
  template <typename Char> auto mid_mid  (box_t<Char> b) { return b[5]; }
  template <typename Char> auto mid_col  (box_t<Char> b) { return b[6]; }
  template <typename Char> auto mid_right(box_t<Char> b) { return b[7]; }
  template <typename Char> auto row_left (box_t<Char> b) { return b[8]; }
  template <typename Char> auto row_mid  (box_t<Char> b) { return b[9]; }
  template <typename Char> auto row_col  (box_t<Char> b) { return b[10]; }
  template <typename Char> auto row_right(box_t<Char> b) { return b[11]; }
  template <typename Char> auto bottom_left (box_t<Char> b) { return b[12]; }
  template <typename Char> auto bottom_mid  (box_t<Char> b) { return b[13]; }
  template <typename Char> auto bottom_col  (box_t<Char> b) { return b[14]; }
  template <typename Char> auto bottom_right(box_t<Char> b) { return b[15]; }

  // clang-format on
} // namespace rich

namespace rich::box {
// S1, S5, S9 and S13 must be exactly one character.
// Others must be zero or one character.
#define RICH_DEFINE_BOX(Name, S0, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10,     \
                        S11, S12, S13, S14, S15)                               \
  namespace detail {                                                           \
    template <typename Char>                                                   \
    inline constexpr std::array Name##Instance{                                \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S0)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S1)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S2)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S3)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S4)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S5)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S6)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S7)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S8)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S9)),              \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S10)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S11)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S12)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S13)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S14)),             \
      std::basic_string_view<Char>(RICH_TYPED_LITERAL(Char, S15)),             \
    };                                                                         \
  }                                                                            \
  template <typename Char>                                                     \
  inline constexpr auto Name = std::span(detail::Name##Instance<Char>)

  // clang-format off

  // ╭─┬╮
  // │ ││
  // ├─┼┤
  // ╰─┴╯
  RICH_DEFINE_BOX(Rounded, "╭", "─", "┬", "╮",
                           "│", " ", "│", "│",
                           "├", "─", "┼", "┤",
                           "╰", "─", "┴", "╯");

  // ╭──╮
  // │  │
  // │  │
  // ╰──╯
  RICH_DEFINE_BOX(RoundedNoSep, "╭", "─", "─", "╮",
                                "│", " ", " ", "│",
                                "│", " ", " ", "│",
                                "╰", "─", "─", "╯");

  // ┌─┬┐
  // │ ││
  // ├─┼┤
  // └─┴┘
  RICH_DEFINE_BOX(Square, "┌", "─", "┬", "┐",
                          "│", " ", "│", "│",
                          "├", "─", "┼", "┤",
                          "└", "─", "┴", "┘");

  // ╭──╮
  // │  │
  // │  │
  // ╰──╯
  RICH_DEFINE_BOX(SquareNoSep, "┌", "─", "─", "┐",
                               "│", " ", " ", "│",
                               "│", " ", " ", "│",
                               "└", "─", "─", "┘");

  // _ __
  // _ __
  // _ __
  // _ __
  RICH_DEFINE_BOX(NoBorder, "", " ", "", "",
                            "", " ", "", "",
                            "", " ", "", "",
                            "", " ", "", "");

  // ● _↵
  // _ _↵
  // _ _↵
  // _ _↵
  RICH_DEFINE_BOX(Editor, "●", " ", "", "↵",
                          "", " ", "", "↵",
                          "", " ", "", "↵",
                          "", " ", "", "↵");

  // clang-format on
#undef RICH_DEFINE_BOX
} // namespace rich::box
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/box.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/cell.hpp
/// @file cell.hpp

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/line_formatter.hpp
/// @file line_formatter.hpp

namespace rich {

  // line_formatter
  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/format/formatter.h#L35-L40

  template <class T, class Char>
  struct line_formatter {
    line_formatter() = delete;
    line_formatter(const line_formatter&) = delete;
    line_formatter& operator=(const line_formatter&) = delete;
  };

  // line_formattable
  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/__format/concepts.h#L39-L47

  template <class Char>
  using fmt_iter_for = Char*;

  // clang-format off
  template <class T>
  concept line_formattable_impl =
    std::copyable<line_formatter<T, typename T::char_type>>
    and std::constructible_from<line_formatter<T, typename T::char_type>, const T&>
    and contextually_convertible_to_bool<line_formatter<T, typename T::char_type>>
    and requires(line_formatter<T, typename T::char_type> f,
                 fmt_iter_for<typename T::char_type> out,
                 const std::size_t n) {
    { std::as_const(f).formatted_size() } -> std::same_as<std::size_t>;
    { f.format_to(out, n) } -> std::same_as<fmt_iter_for<typename T::char_type>>;
  };
  // clang-format on

  template <class T>
  concept line_formattable = line_formattable_impl<std::remove_cvref_t<T>>;

  // line_formattable_default_formatter

  template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
  struct line_formattable_default_formatter {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const L& l, FormatContext& ctx) const -> decltype(ctx.out()) {
      auto out = ctx.out();
      char dlm = '\0';
      for (line_formatter<L, Char> line_fmtr(l); bool(line_fmtr);) {
        out = fmt::detail::write(out, std::exchange(dlm, '\n'));
        out = line_fmtr.format_to(out);
      }
      return out;
    }
  };

  // line_format_to

  inline constexpr auto line_formatter_npos = std::size_t(-1);

  std::size_t npos_sub(std::size_t x, std::size_t y) noexcept {
    if (x == line_formatter_npos) {
      assert(y != line_formatter_npos);
      return x;
    }
    return sat_sub(x, y);
  }

  template <typename Char, std::output_iterator<const Char&> Out, class L>
  Out line_format_to(Out out, const fmt::text_style& style,
                     line_formatter<L, Char>& line_fmtr,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    if (width == line_formatter_npos)
      return line_fmtr.format_to(out);

    const auto fillwidth = sat_sub(width, line_fmtr.formatted_size());
    const auto [left, right] = padding_size(align, fillwidth);
    out = padded_format_to<Char>(out, style, "", fill, 0, left);
    out = line_fmtr.format_to(out, width);
    out = padded_format_to<Char>(out, style, "", fill, 0, right);
    return out;
  }

  template <typename Char, std::output_iterator<const Char&> Out>
  Out line_format_to(Out out, const fmt::text_style& style,
                     std::basic_string_view<Char> sv,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    if (width == line_formatter_npos)
      return padded_format_to<Char>(out, style, sv, fill, 0, 0);

    sv = sv.substr(0, width);
    const auto fillwidth = width - sv.size();
    return aligned_format_to<Char>(out, style, sv, fill, align, fillwidth);
  }

  template <typename Char, std::output_iterator<const Char&> Out, class T>
  Out line_format_to(Out out, const fmt::text_style& style, const T& t,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {
    const auto str = fmt::format("{}", t);
    return line_format_to(out, style, std::basic_string_view<Char>(str), fill,
                          align, width);
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/line_formatter.hpp

namespace rich {
  template <typename Char>
  struct cell {
  private:
    std::shared_ptr<void> ptr_ = nullptr;
    std::any lfmtr_{};
    using handler_t = void(std::any*, bool*, std::size_t*, const std::size_t*,
                           erased_output<Char>*);
    handler_t* handler_ = nullptr;

    void call(bool* b, std::size_t* size = nullptr,
              const std::size_t* in = nullptr,
              erased_output<Char>* str = nullptr) const {
      handler_(&const_cast<std::any&>(lfmtr_), b, size, in, str);
    }

  public:
    cell() = default;

    // NOTE: implicit conversion is allowed
    template <line_formattable L, class D = std::remove_cvref_t<L>,
              class LF = line_formatter<D, Char>>
    requires (not std::same_as<std::remove_cvref_t<L>, cell>)
    cell(L&& l)
      : ptr_(std::make_shared<D>(std::forward<L>(l))),
        lfmtr_(std::make_any<LF>(*std::static_pointer_cast<D>(ptr_))),
        handler_([](std::any* lfmtr, bool* b, std::size_t* size,
                    const std::size_t* in, erased_output<Char>* erased) {
          if (b != nullptr) {
            *b = bool(*std::any_cast<const LF>(lfmtr));
          } else if (size != nullptr) {
            *size = std::any_cast<const LF>(lfmtr)->formatted_size();
          } else {
            assert(in != nullptr and erased != nullptr);
            *erased = std::any_cast<LF>(lfmtr)->format_to(*erased, *in);
          }
        }) {}

    explicit operator bool() const {
      assert(lfmtr_.has_value());
      bool b;
      call(&b);
      return b;
    }

    std::size_t formatted_size() const {
      assert(lfmtr_.has_value());
      std::size_t size;
      call(nullptr, &size);
      return size;
    }

    template <std::output_iterator<const Char&> Out>
    Out format_to(Out out, const std::size_t n = line_formatter_npos) {
      erased_output<Char> erased(out);
      call(nullptr, nullptr, &n, &erased);
      out = rich::out<Out>(erased);
      return out;
    }
  };
} // namespace rich

template <typename Char>
struct rich::line_formatter<rich::cell<Char>, Char> {
private:
  cell<Char> cell_{};

public:
  line_formatter() = default;

  explicit line_formatter(const cell<Char>& l) : cell_(l) {}

  constexpr explicit operator bool() const { return bool(cell_); }

  constexpr std::size_t formatted_size() const {
    return cell_.formatted_size();
  }

  template <std::output_iterator<const Char&> Out>
  Out format_to(Out out, const std::size_t n = line_formatter_npos) {
    return cell_.format_to(out, n);
  }
};

template <typename Char>
struct fmt::formatter<rich::cell<Char>, Char>
  : rich::line_formattable_default_formatter<rich::cell<Char>, Char> {};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/cell.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/enumerate.hpp
/// @file enumerate.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/format_spec.hpp
/// @file format_spec.hpp

namespace rich {
  template <typename Char>
  struct format_spec {
    fmt::text_style style{};
    std::basic_string_view<Char> fill{};
    align_t align{};
    std::size_t width{};
  };

  // `sv` must be zero or one character.
  template <typename Char, std::output_iterator<const Char&> Out>
  Out spec_format_to(Out out, const format_spec<Char>& fs,
                     std::basic_string_view<Char> sv) {
    auto width = sv.empty() ? fs.width : sat_sub(fs.width, 1);
    return aligned_format_to(out, fs.style, sv, fs.fill, fs.align, width);
  }

  // `sv` must be zero or one character.
  template <typename Char, std::output_iterator<const Char&> Out>
  Out rspec_format_to(Out out, const format_spec<Char>& fs,
                      std::basic_string_view<Char> sv) {
    auto width = sv.empty() ? fs.width : sat_sub(fs.width, 1);
    return reversed_format_to(out, fs.style, sv, fs.fill, fs.align, width);
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/format_spec.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/lines.hpp
/// @file lines.hpp

//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/segment.hpp
/// @file segment.hpp


namespace rich {
  template <typename Char>
  struct segment {
  private:
    using string_view_type = std::basic_string_view<Char>;
    string_view_type text_{};
    fmt::text_style style_{};

  public:
    using char_type = Char;

    // ctor
    segment() = default;
    constexpr explicit segment(string_view_type t) : text_(t) {}
    constexpr segment(string_view_type t, const fmt::text_style& s)
      : text_(t), style_(s) {}

    // observer
    auto& text() { return text_; }
    auto text() const { return text_; }
    auto& style() { return style_; }
    const auto& style() const { return style_; }
  };

  template <class T>
  struct is_segment : std::false_type {};

  template <typename Char>
  struct is_segment<segment<Char>> : std::true_type {};

  template <class T>
  inline constexpr bool is_segment_v = is_segment<T>::value;
} // namespace rich

template <typename Char>
struct fmt::formatter<rich::segment<Char>, Char> {
private:
  fmt::formatter<std::basic_string_view<Char>, Char> fmtr{};

public:
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return fmtr.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const rich::segment<Char>& seg, FormatContext& ctx) const
    -> decltype(ctx.out()) {
    auto out = ctx.out();
    const auto [out2, has_style] = rich::set_style<Char>(out, seg.style());
    ctx.advance_to(out2);
    out = fmtr.format(seg.text(), ctx);
    if (has_style)
      out = rich::reset_style<Char>(out);
    return out;
  }
};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/segment.hpp

namespace rich {
  template <class R>
  concept line_range =
    std::ranges::range<R> and is_segment_v<std::ranges::range_value_t<R>>;

  template <class Out1, std::output_iterator<std::ptrdiff_t> Out2, line_range R>
  requires std::output_iterator<Out1, std::ranges::range_value_t<R>>
    std::ptrdiff_t split_newline(Out1 out1, Out2 out2, R&& segs) {
    *out2++ = 0;
    std::ptrdiff_t out1_count = 0;
    const auto npos = std::basic_string_view<
      typename std::ranges::range_value_t<R>::char_type>::npos;

    for (const auto& seg : segs) {
      for (std::size_t current = 0; current < seg.text().size();) {
        auto next = seg.text().find('\n', current);
        *out1++ = {seg.text().substr(current, next - current), seg.style()};
        ++out1_count;
        if (next == npos)
          break;
        *out2++ = out1_count;
        current = next + 1;
      }
    }

    *out2++ = out1_count;
    return out1_count;
  }

  template <typename Char = char>
  struct lines {
  private:
    std::vector<segment<Char>> segments_{};
    // default constructed with vecotor of size 1, value 0
    std::vector<std::ptrdiff_t> bounds_{0};

    struct iterator {
    private:
      const lines* parent_ = nullptr;
      std::ptrdiff_t current_ = 0;

    public:
      using value_type = std::span<const segment<Char>>;
      using difference_type = std::ptrdiff_t;
      using reference = value_type;
      using iterator_category = std::forward_iterator_tag;
      using iterator_concept = std::forward_iterator_tag;

      iterator() = default;
      constexpr iterator(const lines& l, const std::ptrdiff_t n)
        : parent_(std::addressof(l)), current_(n) {}

      bool operator==(const iterator& x) const {
        return parent_ == x.parent_ and current_ == x.current_;
      };
      bool operator!=(const iterator& x) const { return !(*this == x); }

      reference operator*() const {
        assert(parent_ != nullptr);
        auto fst = std::ranges::begin(parent_->segments_);
        return std::span(
          fst + rich::ranges::index(parent_->bounds_, current_),
          fst + rich::ranges::index(parent_->bounds_, current_ + 1));
      }

      iterator& operator++() {
        ++current_;
        return *this;
      }
      iterator operator++(int) {
        iterator t(*this);
        ++(*this);
        return t;
      }
    };

  public:
    using char_type = Char;

    // ctor
    lines() = default;

    // NOTE: implicit conversion is allowed
    template <line_range R>
    constexpr lines(R&& segs, const std::size_t size_hint = 0)
      : bounds_(make_reserved<std::vector<std::ptrdiff_t>>(size_hint + 1)) {
      if constexpr (std::ranges::sized_range<R>)
        segments_.reserve(std::ranges::size(segs) + size_hint);
      else
        segments_.reserve(size_hint + 1);

      split_newline(std::back_inserter(segments_), std::back_inserter(bounds_),
                    segs);
      segments_.shrink_to_fit();
      bounds_.shrink_to_fit();
    }

    constexpr lines(std::initializer_list<segment<Char>> segs,
                    const std::size_t size_hint = 0)
      : bounds_(make_reserved<std::vector<std::ptrdiff_t>>(size_hint + 1)) {
      segments_.reserve(segs.size() + size_hint);

      split_newline(std::back_inserter(segments_), std::back_inserter(bounds_),
                    segs);
      segments_.shrink_to_fit();
      bounds_.shrink_to_fit();
    }

    // observer
    iterator begin() const { return {*this, 0}; }
    iterator end() const { return {*this, std::ssize(bounds_) - 1}; }
    auto empty() const { return std::ranges::size(bounds_) == 1; }
    auto size() const { return std::ranges::size(bounds_) - 1; }
  };

  template <line_range R>
  lines(R&&, const std::size_t = 0)
    -> lines<typename std::ranges::range_value_t<R>::char_type>;

  template <class Out, line_range R>
  requires std::output_iterator<Out, std::ranges::range_value_t<R>> std::size_t
  crop_line(Out out, R&& line, const std::size_t n) {
    std::size_t current = 0;
    for (const auto& seg : line) {
      std::size_t next = current + seg.text().size();
      if (next > n) {
        *out++ = {seg.text().substr(0, n - current), seg.style()};
        return n;
      }
      *out++ = {seg.text(), seg.style()};
      current = next;
    }
    return current;
  }
} // namespace rich

template <typename Char>
struct rich::line_formatter<rich::lines<Char>, Char> {
private:
  const lines<Char>* ptr_ = nullptr;
  std::ranges::iterator_t<lines<Char>> current_{};

public:
  explicit line_formatter(const lines<Char>& l)
    : ptr_(std::addressof(l)), current_(std::ranges::begin(l)) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and current_ != std::ranges::end(*ptr_);
  }

  constexpr std::size_t formatted_size() const {
    return rich::ranges::accumulate(
      *current_, icast<std::size_t>(0), {},
      [](const auto& seg) { return seg.text().size(); });
  }

  template <std::output_iterator<const Char&> Out>
  Out format_to(Out out, const std::size_t n = line_formatter_npos) {
    assert(ptr_ != nullptr);
    auto line = *current_++;
    if (n == line_formatter_npos)
      return fmt::format_to(out, "{}", fmt::join(line, ""));

    auto cropped =
      make_reserved<std::vector<segment<Char>>>(std::ranges::size(line));
    crop_line(std::back_inserter(cropped), line, n);
    return fmt::format_to(out, "{}", fmt::join(cropped, ""));
  }
};

template <typename Char>
struct fmt::formatter<rich::lines<Char>, Char>
  : rich::line_formattable_default_formatter<rich::lines<Char>, Char> {};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/lines.hpp

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/syntax.py#L262-L277
  template <line_formattable L>
  struct enumerate {
    using char_type = typename L::char_type;
    L contents{};
    std::size_t start_line = 1;
    std::size_t end_line = start_line;
    fmt::text_style number_highlight_style = {};
    format_spec<char_type> number_spec{
      .style = fmt::emphasis::faint,
      .fill = " ",
      .align = align_t::right,
      .width = 0,
    };
    std::size_t highlight_line = 0;
    std::basic_string_view<char_type> highlight_char = "❯";
    fmt::text_style highlight_style = fg(fmt::terminal_color::red);
    format_spec<char_type> highlight_spec{
      .style = number_spec.style,
      .fill = number_spec.fill,
      .align = align_t::left,
      .width = 0,
    };

    enumerate() = default;
    constexpr explicit enumerate(const L& l, int = {}) : contents(l) {}
    constexpr explicit enumerate(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  enumerate(R&&, int = {})
    -> enumerate<lines<typename std::ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
struct rich::line_formatter<rich::enumerate<L>, Char> {
private:
  const rich::enumerate<L>* ptr_ = nullptr;
  std::size_t current_ = 1;
  std::size_t number_width_ = 0;
  line_formatter<L, Char> line_fmtr_;

public:
  explicit line_formatter(const rich::enumerate<L>& l)
    : ptr_(std::addressof(l)), current_(l.start_line), number_width_([&l] {
        std::size_t w = std::max(l.start_line, l.end_line);
        w = ilog10(w) + 1;
        return std::max(w, l.number_spec.width);
      }()),
      line_fmtr_(l.contents) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and line_fmtr_;
  }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    const auto highlight_width = ptr_->highlight_spec.width;
    return sat_add(highlight_width + number_width_ + 1,
                   line_fmtr_.formatted_size());
  }

  template <std::output_iterator<const Char&> Out>
  Out format_to(Out out, const std::size_t n = line_formatter_npos) {
    assert(ptr_ != nullptr);
    const auto& ns = ptr_->number_spec;
    const auto& hs = ptr_->highlight_spec;
    const auto contents_width = npos_sub(n, hs.width + number_width_ + 1);
    const auto current = current_++;
    // clang-format off
    if (current == ptr_->highlight_line){
      const auto& c = ptr_->highlight_char;
      out = aligned_format_to<Char>(out, ptr_->highlight_style, c, hs.fill, hs.align, npos_sub(hs.width, not c.empty()));
      out = line_format_to<Char>(out, ptr_->number_highlight_style, current, ns.fill, ns.align, number_width_);
    } else {
      out = spec_format_to<Char>(out, hs, "");
      out = line_format_to<Char>(out, ns.style, current, ns.fill, ns.align, number_width_);
    }
    *out++ = ' ';
    out = line_fmtr_.format_to(out, contents_width);
    // clang-format on
    return out;
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::enumerate<L>, Char>
  : rich::line_formattable_default_formatter<rich::enumerate<L>, Char> {};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/enumerate.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/panel.hpp
/// @file panel.hpp

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/panel.py#L38-L53
  template <line_formattable L>
  struct panel {
    using char_type = typename L::char_type;
    L contents{};
    box_t<char_type> box = box::Rounded<char_type>;
    format_spec<char_type> contents_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = mid_mid(box),
      .align = align_t::left,
      .width = 80,
    };
    format_spec<char_type> border_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = mid_mid(box),
      .align = align_t::left,
      .width = 2,
    };
    std::basic_string_view<char_type> title{};
    bool nomatter = false;

    panel() = default;
    constexpr explicit panel(const L& l, int = {}) : contents(l) {}
    constexpr explicit panel(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  panel(R&&, int = {})
    -> panel<lines<typename std::ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
struct rich::line_formatter<rich::panel<L>, Char> {
private:
  const rich::panel<L>* ptr_ = nullptr;
  line_formatter<L, Char> line_fmtr_;
  std::uint32_t phase_ = 0;

public:
  explicit line_formatter(const rich::panel<L>& l)
    : ptr_(std::addressof(l)), line_fmtr_(l.contents),
      phase_(l.nomatter ? (line_fmtr_ ? 1 : 2) : 0) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and phase_ != 2;
  }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    return ptr_->contents_spec.width;
  }

  template <std::output_iterator<const Char&> Out>
  Out format_to(Out out, const std::size_t n = line_formatter_npos) {
    assert(ptr_ != nullptr);
    const auto& box = ptr_->box;
    assert(std::ranges::size(box) == std::ranges::size(box::Rounded<Char>));

    // calculate contents_width
    const auto width = std::min(ptr_->contents_spec.width, n);
    const auto contents_width = npos_sub(width, ptr_->border_spec.width * 2);

    switch (phase_) {
    case 0: {
      // ╭─╮ top
      ++phase_;
      auto bs = ptr_->border_spec;
      if (bs.align == align_t::left)
        bs.fill = top_mid(box);
      // clang-format off
      out = spec_format_to<Char>(out, bs, top_left(box));
      out = line_format_to<Char>(out, bs.style, ptr_->title, top_mid(box), align_t::center, contents_width);
      out = rspec_format_to<Char>(out, bs, top_right(box));
      // clang-format on
      return out;
    }
    case 1: {
      if (line_fmtr_) {
        // │ │ mid
        const auto& cs = ptr_->contents_spec;
        const auto& bs = ptr_->border_spec;
        // clang-format off
        out = spec_format_to<Char>(out, bs, mid_left(box));
        out = line_format_to<Char>(out, cs.style, line_fmtr_, cs.fill, cs.align, contents_width);
        out = rspec_format_to<Char>(out, bs, mid_right(box));
        // clang-format on
        if (ptr_->nomatter and not line_fmtr_)
          ++phase_;
      } else {
        // ╰─╯ bottom
        ++phase_;
        auto bs = ptr_->border_spec;
        if (bs.align == align_t::left)
          bs.fill = bottom_mid(box);
        // clang-format off
        out = spec_format_to<Char>(out, bs, bottom_left(box));
        out = line_format_to<Char>(out, bs.style, "", bottom_mid(box), {}, contents_width);
        out = rspec_format_to<Char>(out, bs, bottom_right(box));
        // clang-format on
      }
      return out;
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::panel<L>, Char>
  : rich::line_formattable_default_formatter<rich::panel<L>, Char> {};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/panel.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/segments.hpp
/// @file segments.hpp


namespace rich {
  template <typename Char>
  struct segments {
  private:
    using string_view_type = std::basic_string_view<Char>;
    std::list<segment<Char>> instance_{};

  public:
    using char_type = Char;

    // ctor
    segments() = default;
    explicit segments(string_view_type t) : instance_(1, segment(t)) {}
    segments(string_view_type t, const fmt::text_style& s)
      : instance_(1, segment(t, s)) {}

    // observer
    auto begin() const { return instance_.begin(); }
    auto end() const { return instance_.end(); }
    auto empty() const { return instance_.empty(); }
    auto size() const { return instance_.size(); }

    // operation
    // NOTE: `*this` is mutable
    auto partition_point(const std::size_t offset) {
      std::size_t current = 0;
      auto end = instance_.end();
      for (auto it = instance_.begin(); it != end; ++it) {
        const std::size_t next = current + it->text().size();
        if (next > offset)
          return std::make_pair(it, offset - current);
        current = next;
      }
      return std::make_pair(end, icast<std::size_t>(0));
    }

    // NOTE: returned iterator is not const_iterator
    auto split(const std::size_t offset) {
      auto [it, inner_pos] = partition_point(offset);
      if (inner_pos == 0)
        return it;
      assert(it != instance_.end());
      const auto old_text = it->text();
      it->text() = old_text.substr(inner_pos);
      // 指定した要素の前に直接構築し、直接構築した要素のイテレータを返す
      instance_.emplace(it, old_text.substr(0, inner_pos), it->style());
      // 構築した2要素のうち、後ろの要素を返す
      return it;
    }

    auto set_style(string_view_type rng, const fmt::text_style& style) {
      if (instance_.empty())
        throw runtime_error("`segments` not initialized");
      // イテレータを無効化させないため、後ろを先に分割する
      const auto offset2 = icast<std::size_t>(std::ranges::distance(
        rich::ranges::front(instance_).text().begin(), rng.end()));
      auto last = split(offset2);
      const auto offset1 = icast<std::size_t>(std::ranges::distance(
        rich::ranges::front(instance_).text().begin(), rng.begin()));
      auto first = split(offset1);
      for (; first != last; ++first)
        first->style() = style;
      return first;
    }

    auto add_style(string_view_type rng, const fmt::text_style& style) {
      if (instance_.empty())
        throw runtime_error("`segments` not initialized");
      // イテレータを無効化させないため、後ろを先に分割する
      const auto offset2 = icast<std::size_t>(std::ranges::distance(
        rich::ranges::front(instance_).text().begin(), rng.end()));
      auto last = split(offset2);
      const auto offset1 = icast<std::size_t>(std::ranges::distance(
        rich::ranges::front(instance_).text().begin(), rng.begin()));
      auto first = split(offset1);
      for (; first != last; ++first)
        // スタイルが重複したときに例外を投げる
        first->style() |= style;
      return first;
    }
  };
} // namespace rich

template <typename Char>
struct fmt::formatter<rich::segments<Char>, Char>
  : fmt::formatter<
      fmt::join_view<std::ranges::iterator_t<rich::segments<Char>>,
                     std::ranges::sentinel_t<rich::segments<Char>>, Char>,
      Char> {
  template <typename FormatContext>
  auto format(const rich::segments<Char>& segs, FormatContext& ctx) const
    -> decltype(ctx.out()) {
    using base_type = fmt::formatter<
      fmt::join_view<std::ranges::iterator_t<rich::segments<Char>>,
                     std::ranges::sentinel_t<rich::segments<Char>>, Char>,
      Char>;
    return base_type::format(fmt::join(segs, ""), ctx);
  }
};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/segments.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/syntax_highlight.hpp
/// @file syntax_highlight.hpp

namespace rich::theme {
  template <class T, class... Args>
  constexpr std::array<T, sizeof...(Args)> _make_array(Args&&... args) {
    return {std::forward<Args>(args)...};
  }

  inline constexpr auto Default =
    _make_array<fmt::text_style>(fmt::emphasis::faint,          // comment
                                 fg(fmt::terminal_color::red),  // keyword
                                 fg(fmt::terminal_color::blue), // numeric literal
                                 fg(fmt::terminal_color::blue), // string literal
                                 fg(fmt::color::red)            // invalid
    );
} // namespace rich::theme

namespace rich {
  using theme_t = std::span<const fmt::text_style>;

  struct syntax_highlighter {
  private:
    theme_t theme_ = theme_t(theme::Default);

  public:
    syntax_highlighter() = default;
    constexpr explicit syntax_highlighter(theme_t theme) : theme_(theme) {}

    template <class MatchResult>
    constexpr auto operator()(const MatchResult& match_result) const {
      const auto& [prefix, match_object] = match_result;
      if (not match_object)
        return rich::segment(prefix);
      const auto n = rich::match_find(*match_object);
      if (not n or *n >= std::ranges::size(theme_))
        return rich::segment(prefix, rich::ranges::back(theme_));
      return rich::segment(prefix, rich::ranges::index(theme_, *n));
    }
  };

  auto syntax_highlight(std::string_view sv,
                        theme_t theme = theme_t(theme::Default)) {
    static const std::regex re(
      R"((//.*?\n)|\b(auto|const|int|void|if|else|throw|try|catch|return)\b|(\b\d+\b)|(".*?"))");
    assert(theme.size() >= icast<std::size_t>(re.mark_count()));
    return rich::regex_range(sv, re)
           | std::views::transform(syntax_highlighter(theme));
  }
} // namespace rich
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/syntax_highlight.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/table.hpp
/// @file table.hpp


namespace rich {
  template <typename Char = char>
  struct table {
    using value_type = cell<Char>;
    using char_type = Char;

  private:
    std::vector<value_type> cells_{};

  public:
    box_t<char_type> box = box::Rounded<char_type>;
    format_spec<char_type> contents_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = mid_mid(box),
      .align = align_t::left,
      .width = 80,
    };
    format_spec<char_type> border_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = mid_mid(box),
      .align = align_t::left,
      .width = 2,
    };
    std::basic_string_view<char_type> title{};
    bool nomatter = false;

    table() = default;

    template <line_formattable T>
    requires (not std::same_as<std::remove_cvref_t<T>, table>)
    explicit table(T&& t, int = {}) : cells_{std::forward<T>(t)} {}

    /* template <line_formattable T>
    requires std::same_as<std::remove_cvref_t<T>, table>
    explicit table(T&& t, int) : cells_{std::forward<T>(t)} {} */

    template <line_formattable T, line_formattable... U>
    // clang-format off
    requires (std::same_as<typename std::remove_cvref_t<T>::char_type, typename std::remove_cvref_t<U>::char_type> and ... and (sizeof...(U) > 0))
      // clang-format on
      table(T&& t, U&&... u)
      : cells_(make_reserved<std::vector<value_type>>(1 + sizeof...(U))) {
      push_back(std::forward<T>(t));
      using swallow = std::initializer_list<int>;
      (void)swallow{(void(push_back(std::forward<U>(u))), 0)...};
    }

    auto begin() const { return cells_.begin(); }
    auto end() const { return cells_.end(); }
    auto empty() const { return cells_.empty(); }
    auto size() const { return cells_.size(); }

    void reserve(const std::size_t n) { cells_.reserve(n); }

    void push_back(const value_type& ce) { cells_.push_back(ce); }

    void push_back(value_type&& ce) { cells_.push_back(std::move(ce)); }

    template <class... Args>
    auto& emplace_back(Args&&... args) {
      return cells_.emplace_back(std::forward<Args>(args)...);
    }
  };

  template <line_formattable T, class... U>
  table(T, U...) -> table<typename T::char_type>;
} // namespace rich

template <typename Char, std::same_as<Char> Char2>
struct rich::line_formatter<rich::table<Char>, Char2> {
private:
  using line_formatter_type = rich::line_formatter<cell<Char>, Char>;
  const rich::table<Char>* ptr_ = nullptr;
  std::vector<line_formatter_type> lfmtrs_{};
  std::uint32_t phase_ = 0;
  std::ranges::iterator_t<std::vector<line_formatter_type>> current_ =
    std::ranges::begin(lfmtrs_);

public:
  explicit line_formatter(const rich::table<Char>& l)
    : ptr_(std::addressof(l)), lfmtrs_(std::ranges::size(l)),
      phase_([&l]() -> std::uint32_t {
        if (l.nomatter) {
          // NOTE: algorithmはincludeしない方針
          for (const auto& cell : l) {
            if (cell) {
              return 1;
            }
          }
          return 2;
        }
        return 0;
      }()) {
    // NOTE: algorithmはincludeしない方針
    auto it = std::ranges::begin(l);
    for (auto& lfmtr : lfmtrs_)
      lfmtr = line_formatter_type(*it++);
  }

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and phase_ != 2;
  }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    return ptr_->contents_spec.width;
  }

  template <std::output_iterator<const Char&> Out>
  Out format_to(Out out, const std::size_t n = line_formatter_npos) {
    assert(ptr_ != nullptr);
    const auto& box = ptr_->box;
    assert(std::ranges::size(box) == std::ranges::size(box::Rounded<Char>));

    // calculate contents_width
    const auto width = std::min(ptr_->contents_spec.width, n);
    const auto contents_width = npos_sub(width, ptr_->border_spec.width * 2);

    switch (phase_) {
    case 0: {
      // ╭─┬╮ top
      ++phase_;
      auto bs = ptr_->border_spec;
      if (bs.align == align_t::left)
        bs.fill = top_mid(box);
      // clang-format off
      out = spec_format_to<Char>(out, bs, top_left(box));
      out = line_format_to<Char>(out, bs.style, ptr_->title, top_mid(box), align_t::center, contents_width);
      out = rspec_format_to<Char>(out, bs, top_right(box));
      // clang-format on
      return out;
    }
    case 1: {
      if (*current_) {
        // │ ││ mid
        const auto& cs = ptr_->contents_spec;
        const auto& bs = ptr_->border_spec;
        // clang-format off
        out = spec_format_to<Char>(out, bs, mid_left(box));
        out = line_format_to<Char>(out, cs.style, *current_, cs.fill, cs.align, contents_width);
        out = rspec_format_to<Char>(out, bs, mid_right(box));
        // clang-format on
        if (ptr_->nomatter and not *current_
            and std::ranges::next(current_) == std::ranges::end(lfmtrs_))
          ++phase_;
      } else {
        ++current_;
        if (current_ != std::ranges::end(lfmtrs_)) {
          // ├─┼┤ row
          auto bs = ptr_->border_spec;
          if (bs.align == align_t::left)
            bs.fill = row_mid(box);
          // clang-format off
          out = spec_format_to<Char>(out, bs, row_left(box));
          out = line_format_to<Char>(out, bs.style, "", row_mid(box), {}, contents_width);
          out = rspec_format_to<Char>(out, bs, row_right(box));
          // clang-format on
        } else {
          // ╰─┴╯ bottom
          ++phase_;
          auto bs = ptr_->border_spec;
          if (bs.align == align_t::left)
            bs.fill = bottom_mid(box);
          // clang-format off
          out = spec_format_to<Char>(out, bs, bottom_left(box));
          out = line_format_to<Char>(out, bs.style, "", bottom_mid(box), {}, contents_width);
          out = rspec_format_to<Char>(out, bs, bottom_right(box));
          // clang-format on
        }
      }
      return out;
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename Char, typename Char2>
struct fmt::formatter<rich::table<Char>, Char2>
  : rich::line_formattable_default_formatter<rich::table<Char>, Char2> {};
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style/table.hpp
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/style.hpp
//END_FILE_INCLUDE: /home/runner/work/cpp-rich/cpp-rich/include/rich/rich.hpp

#endif // RICH_SINGLE_HEADER_INCLUDED
