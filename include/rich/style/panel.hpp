/// @file panel.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/style/box.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

namespace rich {
  // https://github.com/Textualize/rich/blob/5d3f600f43796393a2c3e4cb20d807c5cf147f44/rich/panel.py#L38-L53
  template <class L>
  struct panel {
    using char_type = typename L::char_type;
    L contents{};
    box_t<char_type> box = box::Rounded<char_type>;
    format_spec<char_type> contents_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = box[4],
      .align = align_t::left,
      .width = 80,
    };
    format_spec<char_type> border_spec{
      .style = fg(fmt::terminal_color::red),
      .fill = box[4],
      .align = align_t::left,
      .width = 2,
    };
    std::basic_string_view<char_type> title{};

    panel() = default;
    constexpr explicit panel(const L& l, int = {}) : contents(l) {}
    constexpr explicit panel(L&& l, int = {}) : contents(std::move(l)) {}
  };

  template <line_range R>
  panel(R&&, int = {})
    -> panel<lines<typename ranges::range_value_t<R>::char_type>>;
} // namespace rich

template <typename L, typename Char>
requires rich::line_formattable<L, Char>
struct rich::line_formatter<rich::panel<L>, Char> {
private:
  const rich::panel<L>* ptr_ = nullptr;
  std::uint32_t phase_ = 0;
  line_formatter<L, Char> line_fmtr_;

public:
  explicit line_formatter(const rich::panel<L>& l)
    : ptr_(std::addressof(l)), line_fmtr_(l.contents) {}

  constexpr explicit operator bool() const {
    return ptr_ != nullptr and phase_ != 2;
  }
  constexpr bool operator!() const { return !bool(*this); }

  constexpr std::size_t formatted_size() const {
    assert(ptr_ != nullptr);
    return ptr_->contents_spec.width;
  }

  template <ranges::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    assert(ptr_ != nullptr);
    const auto w = std::min(ptr_->contents_spec.width, n);
    assert(w > ptr_->border_spec.width * 2);
    const auto& box = ptr_->box;
    assert(ranges::size(box) == ranges::size(box::Rounded<Char>));

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
          bs.fill = box[7];
        // clang-format off
        out = spec_format_to<Char>(out, bs, box[6]);
        out = line_format_to<Char>(out, bs.style, "", box[7], {}, npos_sub(w, bs.width * 2));
        out = rspec_format_to<Char>(out, bs, box[8]);
        // clang-format on
        return {out, w};
      }

      const auto& cs = ptr_->contents_spec;
      const auto& bs = ptr_->border_spec;
      // clang-format off
      out = spec_format_to<Char>(out, bs, box[3]);
      out = line_format_to<Char>(out, cs.style, line_fmtr_, cs.fill, cs.align, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, box[5]);
      // clang-format on
      return {out, w};
    }
    default:
      RICH_UNREACHABLE();
    }
  }
};

template <typename L, typename Char>
struct fmt::formatter<rich::panel<L>, Char>
  : rich::line_formattable_default_formatter<rich::panel<L>, Char> {};
