/// @file lines.hpp
#pragma once
#include <iterator> // std::back_inserter, std::ssize
#include <string_view>
#include <vector>

#include <rich/format.hpp>
#include <rich/ranges.hpp> // rich::ranges::index, rich::ranges::accumulate
#include <rich/style/line_formatter.hpp>
#include <rich/style/segment.hpp>

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
