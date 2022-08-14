/// @file line_formatter.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/style/segment.hpp>

namespace rich {
  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/format/formatter.h#L35-L40
  template <class T, class Char>
  struct line_formatter {
    line_formatter() = delete;
    line_formatter(const line_formatter&) = delete;
    line_formatter& operator=(const line_formatter&) = delete;
  };

  template <class Char>
  using fmt_iter_for = Char*;

  // https://github.com/llvm/llvm-project/blob/ba79c2a25069f09728625982c424920452fa6b83/libcxx/include/__format/concepts.h#L39-L47
  // clang-format off
  template <class T, class Char = char>
  concept line_formattable =
    std::copyable<line_formatter<std::remove_cvref_t<T>, Char>>
    and std::constructible_from<line_formatter<std::remove_cvref_t<T>, Char>,
                                const std::remove_cvref_t<T>&>
    and boolean_testable<line_formatter<std::remove_cvref_t<T>, Char>>
    and requires(line_formatter<std::remove_cvref_t<T>, Char> f,
                 fmt_iter_for<Char> out) {
    { f.format_to(out) } -> std::same_as<fmt_iter_for<Char>>;
  };
  // clang-format on

  // default formatter for line-formattables
  template <typename L, typename Char>
  requires line_formattable<L, Char>
  struct line_formattable_default_formatter {
  private:
    // fmt::formatter<std::string_view, Char> fmtr{};

  public:
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
      // return fmtr.parse(ctx);
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const L& l, FormatContext& ctx) const -> decltype(ctx.out()) {
      auto out = ctx.out();
      char dlm = '\0';
      for (line_formatter<L, Char> line_fmtr(l); bool(line_fmtr);) {
        out = fmt::detail::write(out, std::exchange(dlm, '\n'));
        out = line_fmtr.format_to(out);
      }
      return out;
    }
  };
} // namespace rich
