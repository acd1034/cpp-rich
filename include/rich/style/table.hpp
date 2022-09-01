/// @file table.hpp
#pragma once
#include <any>
#include <iterator> // std::back_inserter
#include <memory>   // std::shared_ptr
#include <vector>

#include <rich/format.hpp>
#include <rich/math.hpp>
#include <rich/style/box.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

namespace rich {
  template <typename Char>
  struct cell {
  private:
    std::shared_ptr<void> ptr_ = nullptr;
    std::any lfmtr_ptr_{};
    std::add_pointer_t<bool(const std::any&)> has_value_ = nullptr;
    std::add_pointer_t<std::size_t(const std::any&)> formatted_size_ = nullptr;
    std::add_pointer_t<std::pair<std::basic_string<Char>, std::size_t>(
      std::any&, const std::size_t)>
      format_ = nullptr;

  public:
    template <line_formattable L, class D = std::remove_cvref_t<L>,
              class LF = line_formatter<D, Char>>
    explicit cell(L&& l)
      : ptr_(std::make_shared<D>(std::forward<L>(l))),
        lfmtr_ptr_(std::make_any<LF>(*std::static_pointer_cast<D>(ptr_))),
        has_value_([](const std::any& lfmtr_ptr) {
          using DF = std::remove_cvref_t<LF>;
          return bool(std::any_cast<const DF&>(lfmtr_ptr));
        }),
        formatted_size_([](const std::any& lfmtr_ptr) {
          using DF = std::remove_cvref_t<LF>;
          return std::any_cast<const DF&>(lfmtr_ptr).formatted_size();
        }),
        format_([](std::any& lfmtr_ptr, const std::size_t n) {
          std::basic_string<Char> str{};
          using DF = std::remove_cvref_t<LF>;
          auto result =
            std::any_cast<DF&>(lfmtr_ptr).format_to(std::back_inserter(str), n);
          return std::make_pair(std::move(str), result.size);
        }) {}

    explicit operator bool() const {
      assert(lfmtr_ptr_.has_value());
      return (*has_value_)(lfmtr_ptr_);
    }

    std::size_t formatted_size() const {
      assert(lfmtr_ptr_.has_value());
      return (*formatted_size_)(lfmtr_ptr_);
    }

    auto format(const std::size_t n = line_formatter_npos) {
      assert(lfmtr_ptr_.has_value());
      return (*format_)(lfmtr_ptr_, n);
    }

    template <std::output_iterator<const Char&> Out>
    auto format_to(Out out, const std::size_t n = line_formatter_npos)
      -> fmt::format_to_n_result<Out> {
      const auto [str, size] = format(n);
      out = rich::copy_to(out, std::basic_string_view<Char>(str));
      return {out, size};
    }
  };

  template <typename Char, std::output_iterator<const Char&> Out>
  Out line_format_to(Out out, const fmt::text_style& style, cell<Char>& ce,
                     std::basic_string_view<Char> fill, const align_t align,
                     const std::size_t width) {

    if (width == line_formatter_npos)
      return ce.format_to(out).out;

    const auto fillwidth = sat_sub(width, ce.formatted_size());
    if (align == align_t::left) {
      out = ce.format_to(out, width).out;
      out = padded_format_to<Char>(out, style, "", fill, 0, fillwidth);
    } else if (align == align_t::center) {
      const auto left = fillwidth / 2;
      out = padded_format_to<Char>(out, style, "", fill, 0, left);
      out = ce.format_to(out, width).out;
      out = padded_format_to<Char>(out, style, "", fill, 0, fillwidth - left);
    } else {
      out = padded_format_to<Char>(out, style, "", fill, 0, fillwidth);
      out = ce.format_to(out, width).out;
    }
    return out;
  }

  template <typename Char = char>
  struct table {
  private:
    std::vector<cell<Char>> cells_{};

  public:
    using char_type = Char;
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

    auto begin() { return cells_.begin(); }
    auto begin() const { return cells_.begin(); }
    auto end() { return cells_.end(); }
    auto end() const { return cells_.end(); }
    auto empty() const { return cells_.empty(); }
    auto size() const { return cells_.size(); }

    template <class... Args>
    auto& emplace_back(Args&&... args) {
      return cells_.emplace_back(std::forward<Args>(args)...);
    }
  };
} // namespace rich

template <typename Char, std::same_as<Char> Char2>
struct rich::line_formatter<rich::table<Char>, Char2> {
private:
  rich::table<Char> tbl_{};
  std::uint32_t phase_ = 0;
  std::ranges::iterator_t<rich::table<Char>> current_ =
    std::ranges::begin(tbl_);

public:
  explicit line_formatter(const rich::table<Char>& l)
    : tbl_(l), phase_([&l]() -> std::uint32_t {
        if (l.nomatter) {
          for (const auto& cell : l) {
            if (cell) {
              return 1;
            }
          }
          return 2;
        }
        return 0;
      }()) {}

  constexpr explicit operator bool() const { return phase_ != 2; }

  constexpr std::size_t formatted_size() const {
    return tbl_.contents_spec.width;
  }

  template <std::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    const auto w = std::min(tbl_.contents_spec.width, n);
    assert(w > tbl_.border_spec.width * 2);
    const auto& box = tbl_.box;
    assert(std::ranges::size(box) == std::ranges::size(box::Rounded<Char>));

    switch (phase_) {
    case 0: {
      // ╭─┬╮ top
      ++phase_;
      auto bs = tbl_.border_spec;
      if (bs.align == align_t::left)
        bs.fill = top_mid(box);
      // clang-format off
      out = spec_format_to<Char>(out, bs, top_left(box));
      out = line_format_to<Char>(out, bs.style, tbl_.title, top_mid(box), align_t::center, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, top_right(box));
      // clang-format on
      return {out, w};
    }
    case 1: {
      if (*current_) {
        // │ ││ mid
        const auto& cs = tbl_.contents_spec;
        const auto& bs = tbl_.border_spec;
        // clang-format off
        out = spec_format_to<Char>(out, bs, mid_left(box));
        out = line_format_to<Char>(out, cs.style, *current_, cs.fill, cs.align, npos_sub(w, bs.width * 2));
        out = rspec_format_to<Char>(out, bs, mid_right(box));
        // clang-format on
        if (tbl_.nomatter and !*current_
            and std::ranges::next(current_) == std::ranges::end(tbl_))
          ++phase_;
      } else {
        ++current_;
        if (current_ != std::ranges::end(tbl_)) {
          // ├─┼┤ row
          auto bs = tbl_.border_spec;
          if (bs.align == align_t::left)
            bs.fill = row_mid(box);
          // clang-format off
          out = spec_format_to<Char>(out, bs, row_left(box));
          out = line_format_to<Char>(out, bs.style, "", row_mid(box), {}, npos_sub(w, bs.width * 2));
          out = rspec_format_to<Char>(out, bs, row_right(box));
          // clang-format on
        } else {
          // ╰─┴╯ bottom
          ++phase_;
          auto bs = tbl_.border_spec;
          if (bs.align == align_t::left)
            bs.fill = bottom_mid(box);
          // clang-format off
          out = spec_format_to<Char>(out, bs, bottom_left(box));
          out = line_format_to<Char>(out, bs.style, "", bottom_mid(box), {}, npos_sub(w, bs.width * 2));
          out = rspec_format_to<Char>(out, bs, bottom_right(box));
          // clang-format on
        }
      }
      return {out, w};
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename Char, typename Char2>
struct fmt::formatter<rich::table<Char>, Char2>
  : rich::line_formattable_default_formatter<rich::table<Char>, Char2> {};
