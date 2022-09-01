/// @file cell.hpp
#pragma once
#include <any>
#include <iterator> // std::back_inserter
#include <memory>   // std::shared_ptr

#include <rich/format.hpp>
#include <rich/style/line_formatter.hpp>

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
} // namespace rich
