#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <rich/exception.hpp>
#include <rich/file.hpp>
#include <rich/regex.hpp>

inline constexpr std::string_view
  hline("\n===================================================================="
        "===========");

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
    static_assert(ranges::forward_range<decltype(rich::regex_range(sv, re))>);
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
