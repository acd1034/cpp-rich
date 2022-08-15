/// @file lines.hpp
#pragma once
#include <iterator> // std::back_inserter, std::ssize
#include <string_view>
#include <vector>

#include <rich/format.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  template <class R>
  concept line_range =
    _ranges::range<R> and is_segment_v<_ranges::range_value_t<R>>;

  template <class T>
  auto reserved_vector(const std::size_t n) {
    std::vector<T> ret;
    ret.reserve(n);
    return ret;
  }

  template <class Out1, _ranges::output_iterator<std::ptrdiff_t> Out2,
            line_range R>
  requires _ranges::output_iterator<Out1, _ranges::range_value_t<R>>
  auto split_newline(Out1 out1, Out2 out2, R&& segs) {
    *out2++ = 0;
    std::ptrdiff_t out1_count = 0;

    for (const auto& seg : segs) {
      for (std::size_t current = 0; current < seg.text().size();) {
        auto next = seg.text().find('\n', current);
        *out1++ = {seg.text().substr(current, next - current), seg.style()};
        ++out1_count;
        if (next == std::string_view::npos)
          break;
        *out2++ = out1_count;
        current = next + 1;
      }
    }

    *out2++ = out1_count;
    return out1_count;
  }

  template <typename Char>
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
        auto fst = _ranges::begin(parent_->segments_);
        return std::span(fst + _ranges::index(parent_->bounds_, current_),
                         fst + _ranges::index(parent_->bounds_, current_ + 1));
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
      : bounds_(reserved_vector<std::ptrdiff_t>(size_hint + 1)) {
      if constexpr (_ranges::sized_range<R>)
        segments_.reserve(_ranges::size(segs) + size_hint);
      else
        segments_.reserve(size_hint + 1);

      split_newline(std::back_inserter(segments_), std::back_inserter(bounds_),
                    segs);
      segments_.shrink_to_fit();
      bounds_.shrink_to_fit();
    }

    // observer
    iterator begin() const { return {*this, 0}; }
    iterator end() const { return {*this, std::ssize(bounds_) - 1}; }
    auto empty() const { return _ranges::size(bounds_) == 1; }
    auto size() const { return _ranges::size(bounds_) - 1; }
  };

  template <line_range R>
  lines(R&&, const std::size_t = 0)
    -> lines<typename _ranges::range_value_t<R>::char_type>;

  template <class Out, line_range R>
  requires _ranges::output_iterator<Out, _ranges::range_value_t<R>>
  auto crop_line(Out out, R&& line, const std::size_t n) {
    std::size_t current = 0;
    for (const auto& seg : line) {
      auto next = current + seg.text().size();
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
  _ranges::iterator_t<lines<Char>> current_{};

public:
  explicit line_formatter(const lines<Char>& l)
    : ptr_(std::addressof(l)), current_(_ranges::begin(l)) {}

  operator bool() const {
    return ptr_ != nullptr and current_ != _ranges::end(*ptr_);
  }
  bool operator!() const { return !bool(*this); }

  template <_ranges::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    auto line = *current_++;
    auto segments = reserved_vector<segment<Char>>(_ranges::size(line));
    auto size = crop_line(std::back_inserter(segments), line, n);
    segments.shrink_to_fit();
    return {fmt::format_to(out, "{}", fmt::join(segments, "")), size};
  }
};

template <typename Char>
struct fmt::formatter<rich::lines<Char>, Char>
  : rich::line_formattable_default_formatter<rich::lines<Char>, Char> {};
