#include <ranges> // std::views::transform
#include <catch2/catch_test_macros.hpp>

#include <rich/file.hpp>
#include <rich/regex.hpp>
#include <rich/style.hpp>

inline constexpr std::string_view hline =
  "============================================================================"
  "===";

static_assert(std::output_iterator<rich::erased_output<char>, const char&>);

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

// This is a comment. Some keywords such as `auto` are contained.
void fn() { throw rich::runtime_error("Rich exception thrown!"); }

TEST_CASE("style", "[style][file]") {
  {
    try {
      fn();
    } catch (rich::exception& e) {
      auto contents = rich::get_file_contents(e.where().file_name());
      const std::size_t extra = 3;
      auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                    e.where().line(), extra);
      auto highlighted = rich::syntax_highlight(partial);
      { // regex_range
        fmt::print("{}\n{}\n", hline, fmt::join(highlighted, ""));
      }
      auto lns = rich::lines(highlighted);
      { // lines
        fmt::print("{}\n{}\n", hline, lns);
      }
      { // panel
        auto pnl = rich::panel(lns);
        pnl.title = std::string_view("Traceback (most recent call)");
        fmt::print("{}\n{}\n", hline, pnl);
      }
      { // enumerate
        auto enm = rich::enumerate(lns);
        enm.start_line = e.where().line() - extra;
        enm.end_line = e.where().line() + extra;
        enm.highlight_line = e.where().line();
        enm.highlight_spec.width = 2;
        fmt::print("{}\n{}\n", hline, enm);
      }
      { // table
        auto tbl = rich::table<char>();
        auto out = std::back_inserter(tbl);
        *out++ = lns;
        *out++ = lns;
        tbl.title = std::string_view("Traceback (most recent call)");
        fmt::print("{}\n{}\n", hline, tbl);
      }
      { // table tuple-like ctor
        auto tbl = rich::table(lns, lns);
        tbl.title = std::string_view("Traceback (most recent call)");
        fmt::print("{}\n{}\n", hline, tbl);
      }
    }
  }
}

TEST_CASE("style", "[style]") {
  { // conversion initializer_list ??? lines ??? panel
    auto sv = std::string_view("Hello world!");
    auto pnl = rich::panel(rich::lines<char>{{sv, {}}});
    fmt::print("{}\n{}\n", hline, pnl);
  }
  { // ctor of table
    auto sv = std::string_view("Hello world!");
    auto lns = rich::lines<char>{{sv, {}}};
    fmt::print("{}\n", hline);
    {
      auto tbl = rich::table(lns);
      fmt::print("{}\n", tbl);
    }
    {
      auto tbl = rich::table(lns, {});
      fmt::print("{}\n", tbl);
    }
    {
      auto tbl = rich::table(lns, lns);
      auto tbl2 = rich::table(tbl);
      fmt::print("{}\n", tbl2);
    }
    /* {
      auto tbl = rich::table(lns, lns);
      auto tbl2 = rich::table(tbl, {});
      fmt::print("{}\n", tbl2); // bus error
    } */
    /* {
      auto tbl = rich::table(lns, lns);
      auto tbl2 = rich::table<char>();
      tbl2.push_back(tbl); // SIGSEGV - Segmentation violation signal
      fmt::print("{}\n", tbl2);
    } */
  }
}
// TEST_CASE("style", "[style][squared]") {}
