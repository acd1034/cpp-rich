/// @file panel.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/style/box.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

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
      // ????????? top
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
        // ??? ??? mid
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
        // ????????? bottom
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
