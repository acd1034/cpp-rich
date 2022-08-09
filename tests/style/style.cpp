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
  auto ts = fmt::emphasis::faint;
  auto ts2 = fg(fmt::terminal_color::red) | bg(fmt::terminal_color::magenta)
             | fmt::emphasis::bold;
  // auto ts3 = fg(fmt::terminal_color::blue) | bg(fmt::terminal_color::cyan) | fmt::emphasis::bold;
  {
    rich::segment seg(orig, ts);
    fmt::print("{}\n", seg);
  }
  {
    auto n = orig.find('8');
    auto m = orig.find('2', n);
    rich::segment seg(orig.substr(n, m - n), ts2);
    fmt::print("{}\n", seg);
  }
  {
    rich::segments segs(orig, ts);
    auto n = orig.find('8');
    segs.set_style(orig.substr(n), ts2);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto n = orig.find('8');
    auto m = orig.find('2', n);
    segs.set_style(orig.substr(n, m - n), ts2);
    fmt::print("{}\n", segs);
  }
}
