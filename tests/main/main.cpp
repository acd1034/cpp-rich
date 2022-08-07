#include <catch2/catch_test_macros.hpp>
#include <iris/iris.hpp>

TEST_CASE("main", "[main][squared]") {
  static_assert(std::is_same_v<decltype(iris::squared(0)), int>);
  CHECK(iris::squared(2) == 4);
}
