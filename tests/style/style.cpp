#include <catch2/catch_test_macros.hpp>

#include <list>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <rich/file.hpp>
#include <rich/regex.hpp>
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

// This is a comment. Some keywords such as `auto` is contained.
void fn() { throw rich::runtime_error("Rich exception thrown!"); }

TEST_CASE("style", "[style][file]") {
  auto s_comment = fmt::emphasis::faint;
  auto s_invalid = fg(fmt::color::red);
  auto s_keyword = fg(fmt::terminal_color::red);
  auto s_string = fg(fmt::terminal_color::blue);
  {
    fmt::print("{}\n", hline);
    try {
      fn();
    } catch (rich::exception& e) {
      auto contents = rich::get_file_contents(e.where().file_name());
      auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                    e.where().line(), 7);
      std::regex re(R"((//.*?\n)|\b(auto|throw|void)\b|(".*?"))");
      auto highlighted =
        rich::regex_range(std::string_view(partial), re)
        | ranges::views::transform(
          [s_comment, s_invalid, s_keyword, s_string](const auto& x) {
            const auto& [pre, mo] = x;
            if (!mo) {
              return rich::segment(pre);
            }
            const auto n = rich::match_find(*mo);
            if (!n) {
              return rich::segment(pre, s_invalid);
            }
            switch (*n) {
            case 0: // comment
              return rich::segment(pre, s_comment);
            case 1: // keyword
              return rich::segment(pre, s_keyword);
            case 2: // string
              return rich::segment(pre, s_string);
            }
            return rich::segment(pre, s_invalid);
          });
      fmt::print("{}:{}:{} in {}\n", e.where().file_name(), e.where().line(),
                 e.where().column(), e.where().function_name());
      fmt::print("{}\n", fmt::join(highlighted, ""));
    }
  }
}

// TEST_CASE("style", "[style][squared]") {}
