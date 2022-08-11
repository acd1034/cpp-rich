/// @file regex.hpp
#pragma once
#include <optional>
#include <regex>
#include <span>
#include <string_view>

#include <rich/fundamental.hpp>

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
  auto group(const std::optional<std::span<const std::sub_match<BiIter>>>& mo,
             const std::size_t n) {
    assert(mo);
    return to_string_view<BiIter, ST>((*mo)[n]);
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
    using value_type =
      std::match_results<typename string_view_type::const_pointer>;
    using difference_type = std::ptrdiff_t;
    // using pointer = const value_type*;
    // using reference = const value_type&;
    using reference =
      std::pair<string_view_type,
                std::optional<decltype(std::span(value_type()))>>;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

  private:
    string_view_type rng_{};
    const regex_type* pregex_ = nullptr;
    std::regex_constants::match_flag_type flags_ =
      std::regex_constants::match_default;
    value_type match_{};
    bool is_prefix_ = false;

    void increment_impl();

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
    // pointer operator->() const { return std::addressof(match_); }

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
  void regex_iterator<ST, Char, Traits>::increment_impl() {
    // flags_ |= std::regex_constants::no_update_pos;
    rng_ = to_string_view(match_.suffix());
    if (to_string_view(match_[0]).empty()) {
      if (rng_.empty()) {
        match_ = value_type();
        return;
      } else if (regex_search(rng_, match_, *pregex_,
                              flags_ | std::regex_constants::match_not_null
                                | std::regex_constants::match_continuous))
        return;
      else
        rng_ = rng_.substr(1);
    }
    flags_ |= std::regex_constants::match_prev_avail;
    if (not regex_search(rng_, match_, *pregex_, flags_))
      match_ = value_type();
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
    increment_impl();
    return *this;
  }

  // regex_range

  template <class ST, class Char, class Traits>
  auto regex_range(std::basic_string_view<Char, ST> sv,
                   const std::basic_regex<Char, Traits>& re,
                   std::regex_constants::match_flag_type flags =
                     std::regex_constants::match_default) {
    return _ranges::subrange(regex_iterator(sv, re, flags),
                             decltype(regex_iterator(sv, re, flags))());
  }
} // namespace rich
