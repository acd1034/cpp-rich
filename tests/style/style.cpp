#include <catch2/catch_test_macros.hpp>
#include <fmt/color.h>
#include <rich/style.hpp>

inline constexpr std::string_view
  hline("\n===================================================================="
        "===========");

TEST_CASE("style", "[style][segment]") {
  std::string_view orig("01234567890123456789");
  auto ts = fg(fmt::terminal_color::blue) | bg(fmt::terminal_color::cyan)
            | fmt::emphasis::bold;
  {
    rich::segment seg(orig, ts);
    fmt::print("{}\n", seg);
  }
  {
    rich::segment seg(orig.substr(3, 10), ts);
    fmt::print("{}\n", seg);
  }
}
