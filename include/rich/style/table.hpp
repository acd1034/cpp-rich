/// @file table.hpp
#pragma once
#include <iterator> // std::back_inserter
#include <memory>   // std::shared_ptr
#include <vector>

#include <rich/format.hpp>
#include <rich/style/box.hpp>
#include <rich/style/format_spec.hpp>
#include <rich/style/line_formatter.hpp>
#include <rich/style/lines.hpp>

namespace rich {
  template <typename Char>
  struct cell {
  private:
    std::shared_ptr<void> lfmtr_ptr_ = nullptr;
    std::add_pointer_t<bool(std::shared_ptr<const void>)> has_value_ = nullptr;
    std::add_pointer_t<std::size_t(std::shared_ptr<const void>)>
      formatted_size_ = nullptr;
    std::add_pointer_t<std::basic_string<Char>(std::shared_ptr<void>,
                                               const std::size_t)>
      format_ = nullptr;

  public:
    template <line_formattable L,
              class LF = line_formatter<std::remove_cvref_t<L>, Char>>
    explicit cell(L&& l)
      : lfmtr_ptr_(std::make_shared<LF>(std::forward<L>(l))),
        has_value_([](std::shared_ptr<const void> lfmtr_ptr) {
          using DF = std::remove_cvref_t<LF>;
          return bool(*std::static_pointer_cast<const DF>(lfmtr_ptr));
        }),
        formatted_size_([](std::shared_ptr<const void> lfmtr_ptr) {
          using DF = std::remove_cvref_t<LF>;
          return std::static_pointer_cast<const DF>(lfmtr_ptr)
            ->formatted_size();
        }),
        format_([](std::shared_ptr<void> lfmtr_ptr, const std::size_t n) {
          std::basic_string<Char> ret{};
          using DF = std::remove_cvref_t<LF>;
          std::static_pointer_cast<DF>(lfmtr_ptr)->format_to(
            std::back_inserter(ret), n);
          return ret;
        }) {}

    explicit operator bool() const {
      assert(lfmtr_ptr_);
      return (*has_value_)(std::static_pointer_cast<const void>(lfmtr_ptr_));
    }

    std::size_t formatted_size() const {
      assert(lfmtr_ptr_);
      return (*formatted_size_)(
        std::static_pointer_cast<const void>(lfmtr_ptr_));
    }

    std::basic_string<Char> format(const std::size_t n = line_formatter_npos) {
      assert(lfmtr_ptr_);
      return (*format_)(lfmtr_ptr_, n);
    }
  };

  template <typename Char = char>
  struct table {
    using char_type = Char;
    std::vector<cell<char_type>> contents{};
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

    template <class... Args>
    cell<char_type>& emplace_back(Args&&... args) {
      return contents.emplace_back(std::forward<Args>(args)...);
    }
  };

  template <line_range R>
  table(R&&, int = {})
    -> table<typename std::ranges::range_value_t<R>::char_type>;
} // namespace rich

template <typename Char, std::same_as<Char> Char2>
struct rich::line_formatter<rich::table<Char>, Char2> {
private:
  rich::table<Char>* ptr_ = nullptr;
  cell<Char>& c = rich::ranges::front(ptr_->contents);
  std::uint32_t phase_ = 0;

public:
  explicit line_formatter(rich::table<Char>& l) : ptr_(std::addressof(l)) {}

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
    assert(std::ranges::size(box) == std::ranges::size(box::Rounded2<Char>));

    switch (phase_) {
    case 0: {
      ++phase_;
      auto bs = ptr_->border_spec;
      if (bs.align == align_t::left)
        bs.fill = box[1];
      // clang-format off
      out = spec_format_to<Char>(out, bs, box[0]);
      out = line_format_to<Char>(out, bs.style, ptr_->title, box[1], align_t::center, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, box[3]);
      // clang-format on
      return {out, w};
    }
    case 1: {
      if (!c) {
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
      auto str = c.format(npos_sub(w, bs.width * 2));
      out = line_format_to<Char>(out, cs.style, std::basic_string_view<Char>(str), cs.fill, cs.align, npos_sub(w, bs.width * 2));
      out = rspec_format_to<Char>(out, bs, box[1*4+3]);
      // clang-format on
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
