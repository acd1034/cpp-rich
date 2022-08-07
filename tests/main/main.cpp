#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <rich/exception.hpp>
#include <rich/fundamental.hpp>

TEST_CASE("main", "[main][squared]") {
  static_assert(std::is_same_v<decltype(rich::squared(0)), int>);
  CHECK(rich::squared(2) == 4);
}

void fn() {
  throw rich::runtime_error("Rich exception thrown!");
}

TEST_CASE("main", "[main][exception]") {
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
