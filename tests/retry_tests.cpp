#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Retry 3 times and return success status", "[retry]") {
  int n = 0;

  // clang-format off
  auto retry =
        bhv::retry("retry", 3)
        .child<bhv::action>("a", [&] { return ++n < 3 ? bhv::status::failure : bhv::status::success; });
  // clang-format on

  REQUIRE((retry() == bhv::status::success && n == 3));
}

TEST_CASE("Retry 3 times and failure", "[retry]") {
  int n = 0;

  // clang-format off
  auto retry =
        bhv::retry("retry", 3)
        .child<bhv::action>("a", [&] { return ++n < 4 ? bhv::status::failure : bhv::status::success; });
  // clang-format on

  REQUIRE((retry() == bhv::status::failure && n == 3));
}

TEST_CASE("Retry 3 times with running node", "[retry]") {
  int n = 0;

  // clang-format off
  auto retry =
        bhv::retry("retry", 3)
        .child<bhv::action>("a", [&] {
          return ++n < 3 ? bhv::status::running :
                 n < 5 ? bhv::status::failure :
                 bhv::status::success;
        });
  // clang-format on

  for (int i = 0; i < 2; ++i) {
    n = 0;
    REQUIRE((retry() == bhv::status::running && n == 1));
    REQUIRE((retry() == bhv::status::running && n == 2));
    REQUIRE((retry() == bhv::status::success && n == 5));
  }
}

TEST_CASE("Retry after the exception", "[retry]") {
  int n = 0;
  bool exception = true;

  // clang-format off
  auto retry =
        bhv::retry("retry", 3)
        .child<bhv::action>("a", [&] {
          if (++n < 3)
            return bhv::status::running;
          if (exception) {
            exception = false;
            throw 42;
          }
          if (n < 5)
            return bhv::status::failure;
          return bhv::status::success;
        });
  // clang-format on

  for (int i = 0; i < 2; ++i) {
    n = 0;
    exception = true;
    REQUIRE((retry() == bhv::status::running && n == 1));
    REQUIRE((retry() == bhv::status::running && n == 2));
    REQUIRE_THROWS(retry());
    REQUIRE((n == 3 && !exception));
    REQUIRE((retry() == bhv::status::success && n == 5));
  }
}
