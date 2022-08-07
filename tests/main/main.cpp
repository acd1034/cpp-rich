#include <catch2/catch_test_macros.hpp>
#include <rich/fundamental.hpp>

TEST_CASE("main", "[main][squared]") {
  static_assert(std::is_same_v<decltype(rich::squared(0)), int>);
  CHECK(rich::squared(2) == 4);
}
