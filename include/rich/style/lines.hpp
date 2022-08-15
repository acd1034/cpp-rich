/// @file lines.hpp
#pragma once
#include <string_view>
#include <vector>

#include <rich/format.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  template <class T>
  auto reserved_vector(const std::size_t n) {
    std::vector<T> ret;
    ret.reserve(n);
    return ret;
  }

  template <_ranges::range R>
  requires std::same_as<_ranges::range_value_t<R>, segment>
  auto split_newline(R&& segs, const std::size_t size_hint = 0) {
    std::vector<segment> segments;
    if constexpr (_ranges::sized_range<R>)
      segments.reserve(_ranges::size(segs) + size_hint);
    else
      segments.reserve(size_hint + 1);
    auto bounds = reserved_vector<std::ptrdiff_t>(size_hint + 1);
    bounds.push_back(0);

    for (const auto& seg : segs) {
      for (std::size_t current = 0; current < seg.text().size();) {
        auto next = seg.text().find('\n', current);
        segments.emplace_back(seg.text().substr(current, next - current),
                              seg.style());
        if (next == std::string_view::npos)
          break;
        bounds.push_back(std::ssize(segments));
        current = next + 1;
      }
    }

    bounds.push_back(std::ssize(segments));
    segments.shrink_to_fit();
    bounds.shrink_to_fit();
    return std::make_pair(std::move(segments), std::move(bounds));
  }

  struct lines {
  private:
    std::vector<segment> segments_{};
    // initialized with vecotor of size 1, value 0
    std::vector<std::ptrdiff_t> bounds_{0};

    struct iterator {
    private:
      const lines* parent_ = nullptr;
      std::ptrdiff_t current_ = 0;

    public:
      using value_type = std::span<const segment>;
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
    }; // struct iterator

  public:
    lines() = default;

    // NOTE: implicit conversion is allowed
    template <_ranges::range R>
    requires std::same_as<_ranges::range_value_t<R>, segment>
    constexpr lines(R&& segs, const std::size_t size_hint = 0) {
      std::tie(segments_, bounds_) = split_newline(segs, size_hint);
    }

    // observer
    iterator begin() const { return {*this, 0}; }
    iterator end() const { return {*this, std::ssize(bounds_) - 1}; }
    auto empty() const { return _ranges::size(bounds_) == 1; }
    auto size() const { return _ranges::size(bounds_) - 1; }
  }; // struct lines

  template <_ranges::output_iterator<segment> Out, _ranges::range R>
  requires std::same_as<_ranges::range_value_t<R>, segment>
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
struct rich::line_formatter<rich::lines, Char> {
private:
  const lines* ptr_ = nullptr;
  _ranges::iterator_t<lines> current_{};

public:
  explicit line_formatter(const lines& l)
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
    auto segments = reserved_vector<segment>(_ranges::size(line));
    auto size = crop_line(std::back_inserter(segments), line, n);
    segments.shrink_to_fit();
    return {fmt::format_to(out, "{}", fmt::join(segments, "")), size};
  }
};

template <typename Char>
struct fmt::formatter<rich::lines, Char>
  : rich::line_formattable_default_formatter<rich::lines, Char> {};
