/// @file table.hpp
#pragma once
#include <vector>

#include <rich/format.hpp>
#include <rich/math.hpp>
#include <rich/style/box.hpp>
#include <rich/style/cell.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

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
