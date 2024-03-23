#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Repeat 3 times and return success status", "[repeat]") {
  int n = 0;

  // clang-format off
  auto repeat =
        bhv::repeat("repeat", 3)
        .child<bhv::action>("a", [&] { ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE((repeat() == bhv::status::success && n == 3));
}

TEST_CASE("Repeat 3 times and return failure", "[repeat]") {
  int n = 0;

  // clang-format off
  auto repeat =
        bhv::repeat("repeat")
        .child<bhv::action>("a", [&] { return ++n < 3 ? bhv::status::success : bhv::status::failure; });
  // clang-format on

  REQUIRE((repeat() == bhv::status::failure && n == 3));
}

TEST_CASE("Repeat 3 times with running node", "[repeat]") {
  int n = 0;

  // clang-format off
  auto repeat =
        bhv::repeat("repeat", 3)
        .child<bhv::action>("a", [&] { return ++n < 3 ? bhv::status::running : bhv::status::success; });
  // clang-format on

  for (int i = 0; i < 2; ++i) {
    n = 0;
    REQUIRE((repeat() == bhv::status::running && n == 1));
    REQUIRE((repeat() == bhv::status::running && n == 2));
    REQUIRE((repeat() == bhv::status::success && n == 5));
  }
}

TEST_CASE("Repeat after the exception", "[repeat]") {
  int n = 0;
  bool exception = true;

  // clang-format off
  auto repeat =
        bhv::repeat("repeat", 3)
        .child<bhv::action>("a", [&] {
          if (++n < 3)
            return bhv::status::running;
          if (exception) {
            exception = false;
            throw 42;
          }
          return bhv::status::success;
        });
  // clang-format on

  for (int i = 0; i < 2; ++i) {
    n = 0;
    exception = true;
    REQUIRE((repeat() == bhv::status::running && n == 1));
    REQUIRE((repeat() == bhv::status::running && n == 2));
    REQUIRE_THROWS(repeat());
    REQUIRE((n == 3 && !exception));
    REQUIRE((repeat() == bhv::status::success && n == 6));
  }
}
