/// @file table.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/style/box.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

namespace rich {
  template <line_formattable L>
  struct table {
    using char_type = typename L::char_type;
    L contents{};
    box_t<char_type> box = box::Rounded2<char_type>;
    format_spec<char_type> contents_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = box[1 * 4 + 1],
      .align = align_t::left,
      .width = 80,
    };
    format_spec<char_type> border_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = box[1 * 4 + 1],
      .align = align_t::left,
      .width = 2,
    };
    std::basic_string_view<char_type> title{};

    table() = default;
    constexpr explicit table(const L& l, int = {}) : contents(l) {}
    constexpr explicit table(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  table(R&&, int = {})
    -> table<lines<typename std::ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <rich::line_formattable L, std::same_as<typename L::char_type> Char>
struct rich::line_formatter<rich::table<L>, Char> {
private:
  const rich::table<L>* ptr_ = nullptr;
  std::uint32_t phase_ = 0;
  line_formatter<L, Char> line_fmtr_;

public:
  explicit line_formatter(const rich::table<L>& l)
    : ptr_(std::addressof(l)), line_fmtr_(l.contents) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and phase_ != 2;
  }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    return ptr_->contents_spec.width;
  }

  template <std::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    const auto w = std::min(ptr_->contents_spec.width, n);
    assert(w > ptr_->border_spec.width * 2);
    const auto& box = ptr_->box;
    // assert(std::ranges::size(box) == std::ranges::size(box::Rounded<Char>));

    switch (phase_) {
    case 0: {
      ++phase_;
      auto bs = ptr_->border_spec;
      if (bs.align == align_t::left)
        bs.fill = box[1];
      // clang-format off
      out = spec_format_to<Char>(out, bs, box[0]);
      out = line_format_to<Char>(out, bs.style, ptr_->title, box[1], align_t::center, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, box[2]);
      // clang-format on
      return {out, w};
    }
    case 1: {
      if (!line_fmtr_) {
        ++phase_;
        auto bs = ptr_->border_spec;
        if (bs.align == align_t::left)
          bs.fill = box[3 * 4 + 1];
        // clang-format off
        out = spec_format_to<Char>(out, bs, box[3*4]);
        out = line_format_to<Char>(out, bs.style, "", box[3*4+1], {}, npos_sub(w, bs.width * 2));
        out = rspec_format_to<Char>(out, bs, box[3*4+3]);
        // clang-format on
        return {out, w};
      }

      const auto& cs = ptr_->contents_spec;
      const auto& bs = ptr_->border_spec;
      // clang-format off
      out = spec_format_to<Char>(out, bs, box[1*4]);
      out = line_format_to<Char>(out, cs.style, line_fmtr_, cs.fill, cs.align, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, box[1*4+3]);
      // clang-format on
      return {out, w};
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::table<L>, Char>
  : rich::line_formattable_default_formatter<rich::table<L>, Char> {};
