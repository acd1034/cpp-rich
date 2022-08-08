#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <rich/exception.hpp>
#include <rich/file.hpp>
#include <rich/fundamental.hpp>

inline constexpr std::string_view
  hline("\n===============================================================================");

TEST_CASE("main", "[main][squared]") {
  static_assert(std::is_same_v<decltype(rich::squared(0)), int>);
  CHECK(rich::squared(2) == 4);
}

void fn() {
  throw rich::runtime_error("Rich exception thrown!");
}

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
  } catch (std::exception& e) { std::cout << e.what() << std::endl; }
}

TEST_CASE("main", "[main][file]") {
  {
    std::cout << hline << std::endl;
    auto contents = rich::get_file_contents(__FILE__);
    auto partial = rich::extract_partial_contents(contents, 23, 7);
    std::cout << partial << std::endl;
  }
  {
    std::cout << hline << std::endl;
    try {
      fn();
    } catch (rich::exception& e) {
      auto contents = rich::get_file_contents(e.where().file_name());
      auto partial = rich::extract_partial_contents(contents, e.where().line(), 7);
      std::cout << partial << std::endl;
    }
  }
}
