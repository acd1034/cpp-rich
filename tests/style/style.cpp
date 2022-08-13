#include <catch2/catch_test_macros.hpp>

#include <rich/file.hpp>
#include <rich/regex.hpp>
#include <rich/style.hpp>

inline constexpr std::string_view
  hline("\n===================================================================="
        "===========");

TEST_CASE("style", "[style][span]") {
  std::string_view orig("01234567890123456789");
  auto ts = fmt::emphasis::faint;
  auto ts2 = fg(fmt::terminal_color::red) | bg(fmt::terminal_color::magenta)
             | fmt::emphasis::bold;
  auto ts3 = fg(fmt::terminal_color::blue) | bg(fmt::terminal_color::cyan)
             | fmt::emphasis::bold;
  // span
  {
    rich::span seg(orig, ts);
    fmt::print("{}\n", seg);
  }
  {
    auto m = orig.find('8');
    auto n = orig.find('2', m);
    rich::span seg(orig.substr(m, n - m), ts2);
    fmt::print("{}\n", seg);
  }
  // segment
  {
    rich::segment segs(orig, ts);
    segs.set_style(orig, ts2);
    CHECK(segs.size() == 1);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto n = orig.find('8');
    segs.set_style(orig.substr(0, n), ts2);
    CHECK(segs.size() == 2);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto n = orig.find('8');
    segs.set_style(orig.substr(n), ts2);
    CHECK(segs.size() == 2);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto m = orig.find('8');
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(m, n - m), ts2);
    CHECK(segs.size() == 3);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, n - l), ts2);
    segs.set_style(orig.substr(l, m - l), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, n - l), ts2);
    segs.set_style(orig.substr(m, n - m), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
    auto l = orig.find('2');
    auto m = orig.find('8', l);
    auto n = orig.find('2', m);
    segs.set_style(orig.substr(l, m - l), ts2);
    segs.set_style(orig.substr(l, n - l), ts3);
    CHECK(segs.size() == 4);
    fmt::print("{}\n", segs);
  }
  {
    rich::segment segs(orig, ts);
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
  std::regex re(R"((//.*?\n)|\b(auto|throw|void)\b|(".*?"))");
  std::vector<fmt::text_style> styles{
    fmt::emphasis::faint,          // comment
    fg(fmt::terminal_color::red),  // keyword
    fg(fmt::terminal_color::blue), // literal
    fg(fmt::color::red),
  };
  {
    fmt::print("{}\n", hline);
    try {
      fn();
    } catch (rich::exception& e) {
      auto contents = rich::get_file_contents(e.where().file_name());
      auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                    e.where().line(), 7);
      auto highlighted =
        rich::regex_range(std::string_view(partial), re)
        | ranges::views::transform([&styles](const auto& x) {
            const auto& [pre, mo] = x;
            if (!mo)
              return rich::span(pre);
            const auto n = rich::match_find(*mo);
            if (!n or *n >= ranges::size(styles))
              return rich::span(pre, ranges::back(styles));
            return rich::span(pre, ranges::index(styles, *n));
          });
      fmt::print("{}:{}:{} in {}\n", e.where().file_name(), e.where().line(),
                 e.where().column(), e.where().function_name());
      fmt::print("{}\n", fmt::join(highlighted, ""));
    }
  }
}

// TEST_CASE("style", "[style][squared]") {}
