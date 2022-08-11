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
  auto ts3 = fg(fmt::terminal_color::blue) | bg(fmt::terminal_color::cyan)
             | fmt::emphasis::bold;
  // segment
  {
    rich::segment seg(orig, ts);
    fmt::print("{}\n", seg);
  }
  {
    auto m = orig.find('8');
    auto n = orig.find('2', m);
    rich::segment seg(orig.substr(m, n - m), ts2);
    fmt::print("{}\n", seg);
  }
  // segments
  {
    rich::segments segs(orig, ts);
    segs.set_style(orig, ts2);
    CHECK(segs.size() == 1);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto n = orig.find('8');
    segs.set_style(orig.substr(0, n), ts2);
    CHECK(segs.size() == 2);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto n = orig.find('8');
    segs.set_style(orig.substr(n), ts2);
    CHECK(segs.size() == 2);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto m = orig.find('8');
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(m, n - m), ts2);
    CHECK(segs.size() == 3);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, n - l), ts2);
    segs.set_style(orig.substr(l, m - l), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, n - l), ts2);
    segs.set_style(orig.substr(m, n - m), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, m - l), ts2);
    segs.set_style(orig.substr(l, n - l), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segments segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(m, n - m), ts2);
    segs.set_style(orig.substr(l, n - l), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
}
