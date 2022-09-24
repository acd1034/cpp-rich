/// @file syntax_highlight.hpp
#pragma once
#include <rich/format.hpp>
#include <rich/regex.hpp>
#include <rich/style/segment.hpp>

namespace rich::theme {
  template <class T, class... Args>
  constexpr std::array<T, sizeof...(Args)> _make_array(Args&&... args) {
    return {std::forward<Args>(args)...};
  }

  inline constexpr auto Default =
    _make_array<fmt::text_style>(fmt::emphasis::faint,          // comment
                                 fg(fmt::terminal_color::red),  // keyword
                                 fg(fmt::terminal_color::blue), // numeric literal
                                 fg(fmt::terminal_color::blue), // string literal
                                 fg(fmt::color::red)            // invalid
    );
} // namespace rich::theme

namespace rich {
  using theme_t = std::span<const fmt::text_style>;

  struct syntax_highlighter {
  private:
    theme_t theme_ = theme_t(theme::Default);

  public:
    syntax_highlighter() = default;
    constexpr explicit syntax_highlighter(theme_t theme) : theme_(theme) {}

    template <class MatchResult>
    constexpr auto operator()(const MatchResult& match_result) const {
      const auto& [prefix, match_object] = match_result;
      if (not match_object)
        return rich::segment(prefix);
      const auto n = rich::match_find(*match_object);
      if (not n or *n >= std::ranges::size(theme_))
        return rich::segment(prefix, rich::ranges::back(theme_));
      return rich::segment(prefix, rich::ranges::index(theme_, *n));
    }
  };

  auto syntax_highlight(std::string_view sv,
                        theme_t theme = theme_t(theme::Default)) {
    static const std::regex re(
      R"((//.*?\n)|\b(auto|const|int|void|if|else|throw|try|catch|return)\b|(\b\d+\b)|(".*?"))");
    assert(theme.size() >= icast<std::size_t>(re.mark_count()));
    return rich::regex_range(sv, re)
           | std::views::transform(syntax_highlighter(theme));
  }
} // namespace rich
