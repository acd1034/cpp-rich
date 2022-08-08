#include <catch2/catch_test_macros.hpp>

#include <list>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <rich/style.hpp>

inline constexpr std::string_view
  hline("\n===================================================================="
        "===========");

TEST_CASE("style", "[style][segment]") {
  std::string_view orig("01234567890123456789");
  auto ts = fg(fmt::terminal_color::red) | bg(fmt::terminal_color::magenta)
            | fmt::emphasis::bold;
  auto ts2 = fg(fmt::terminal_color::blue) | bg(fmt::terminal_color::cyan)
             | fmt::emphasis::bold;
  {
    rich::segment seg(orig, ts);
    fmt::print("{}\n", seg);
  }
  {
    auto n = orig.find('8');
    rich::segment seg(orig.substr(0, n), ts2);
    fmt::print("{}\n", seg);
  }
  {
    std::list<rich::segment> l;
    auto n = orig.find('8');
    l.emplace_back(orig.substr(0, n), ts);
    l.emplace_back(orig.substr(n), ts2);
    fmt::print("{}\n", fmt::join(l, ""));
  }
}
