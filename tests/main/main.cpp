#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <rich/exception.hpp>
#include <rich/file.hpp>
#include <rich/regex.hpp>

inline constexpr std::string_view hline =
  "============================================================================"
  "===";

void fn() { throw rich::runtime_error("Rich exception thrown!"); }

TEST_CASE("main", "[main][exception]") {
  std::cout << hline << std::endl;

  try {
    fn();
  } catch (rich::exception& e) {
    std::cout << e.what() << std::endl;
    std::cout << e.where().line() << std::endl;
    std::cout << e.where().column() << std::endl;
    std::cout << e.where().file_name() << std::endl;
    std::cout << e.where().function_name() << std::endl;
  }

  try {
    fn();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST_CASE("main", "[main][find_nth]") {
  {
    //                     0123456
    std::string_view sv = "a--a--";
    constexpr auto npos = std::string_view::npos;
    CHECK(rich::find_nth(sv, 'a', 0, 0) == 0);
    CHECK(rich::find_nth(sv, 'a', 0, 1) == 1);
    CHECK(rich::find_nth(sv, 'a', 0, 2) == 2);
    CHECK(rich::find_nth(sv, 'a', 0, 3) == 3);
    CHECK(rich::find_nth(sv, 'a', 0, 4) == 4);
    CHECK(rich::find_nth(sv, 'a', 0, 5) == 5);
    CHECK(rich::find_nth(sv, 'a', 0, 6) == 6);

    CHECK(rich::find_nth(sv, 'a', 1, 0) == 0);
    CHECK(rich::find_nth(sv, 'a', 1, 1) == 3);
    CHECK(rich::find_nth(sv, 'a', 1, 2) == 3);
    CHECK(rich::find_nth(sv, 'a', 1, 3) == 3);
    CHECK(rich::find_nth(sv, 'a', 1, 4) == npos);
    CHECK(rich::find_nth(sv, 'a', 1, 5) == npos);
    CHECK(rich::find_nth(sv, 'a', 1, 6) == npos);

    CHECK(rich::find_nth(sv, 'a', 2, 0) == 3);
    CHECK(rich::find_nth(sv, 'a', 2, 1) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 2) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 3) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 4) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 5) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 6) == npos);

    CHECK(rich::find_nth(sv, 'a', 3, 0) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 1) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 2) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 3) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 4) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 5) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 6) == npos);
  }
  {
    //                     0123456
    std::string_view sv = "--a--a";
    constexpr auto npos = std::string_view::npos;
    CHECK(rich::find_nth(sv, 'a', 0, 0) == 0);
    CHECK(rich::find_nth(sv, 'a', 0, 1) == 1);
    CHECK(rich::find_nth(sv, 'a', 0, 2) == 2);
    CHECK(rich::find_nth(sv, 'a', 0, 3) == 3);
    CHECK(rich::find_nth(sv, 'a', 0, 4) == 4);
    CHECK(rich::find_nth(sv, 'a', 0, 5) == 5);
    CHECK(rich::find_nth(sv, 'a', 0, 6) == 6);

    CHECK(rich::find_nth(sv, 'a', 1, 0) == 2);
    CHECK(rich::find_nth(sv, 'a', 1, 1) == 2);
    CHECK(rich::find_nth(sv, 'a', 1, 2) == 2);
    CHECK(rich::find_nth(sv, 'a', 1, 3) == 5);
    CHECK(rich::find_nth(sv, 'a', 1, 4) == 5);
    CHECK(rich::find_nth(sv, 'a', 1, 5) == 5);
    CHECK(rich::find_nth(sv, 'a', 1, 6) == npos);

    CHECK(rich::find_nth(sv, 'a', 2, 0) == 5);
    CHECK(rich::find_nth(sv, 'a', 2, 1) == 5);
    CHECK(rich::find_nth(sv, 'a', 2, 2) == 5);
    CHECK(rich::find_nth(sv, 'a', 2, 3) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 4) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 5) == npos);
    CHECK(rich::find_nth(sv, 'a', 2, 6) == npos);

    CHECK(rich::find_nth(sv, 'a', 3, 0) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 1) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 2) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 3) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 4) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 5) == npos);
    CHECK(rich::find_nth(sv, 'a', 3, 6) == npos);
  }
}

TEST_CASE("main", "[main][file]") {
  {
    std::cout << hline << std::endl;
    auto contents = rich::get_file_contents(__FILE__);
    auto partial =
      rich::extract_partial_contents(std::string_view(contents), 23, 7);
    std::cout << partial << std::endl;
  }
  {
    std::cout << hline << std::endl;
    try {
      fn();
    } catch (rich::exception& e) {
      auto contents = rich::get_file_contents(e.where().file_name());
      auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                    e.where().line(), 7);
      std::cout << partial << std::endl;
    }
  }
}

TEST_CASE("main", "[main][regex]") {
  {
    std::regex re("a+|f+");
    std::string_view sv("aaabcdefffghij");
    static_assert(
      std::ranges::forward_range<decltype(rich::regex_range(sv, re))>);
  }
  {
    std::regex re("(a+)|(f+)");
    std::string_view sv("aaabcdefffghij");
    std::cout << hline << std::endl;
    for (const auto& [pre, mo] : rich::regex_range(sv, re)) {
      if (!mo)
        std::cout << "prefix: " << pre << std::endl;
      else if (auto n = rich::match_find(*mo); !n) {
        std::cout << pre << std::endl;
      } else {
        std::cout << *n << "match: " << rich::match_group(*mo, *n) << std::endl;
      }
    }
  }
  {
    std::regex re("a*|b*|c*");
    std::string_view sv("aaabbbccc");
    std::cout << hline << std::endl;
    for (const auto& [pre, mo] : rich::regex_range(sv, re))
      std::cout << (mo ? "match: " : "prefi: ") << pre << std::endl;
  }
}

// TEST_CASE("main", "[main][squared]") {}
