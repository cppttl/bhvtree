#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Fallback node", "[fallback]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::fallback("root")
        .add<bhv::action>("1", [&n] { n = 42; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { n = 43; return bhv::status::success; })
        .add<bhv::action>("3", [&n] { n = 44; return bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 43);
}

TEST_CASE("Failed fallback node", "[fallback]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::fallback("root")
        .add<bhv::action>("1", [&n] { n = 42; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { n = 43; return bhv::status::failure; })
        .add<bhv::action>("3", [&n] { n = 44; return bhv::status::failure; });
  // clang-format on

  REQUIRE(seq() == bhv::status::failure);
  REQUIRE(n == 44);
}

TEST_CASE("Running failed fallback node", "[fallback]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::fallback("root")
        .add<bhv::action>("1", [&n] { n = 1; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { return ++n < 3 ? bhv::status::running: bhv::status::failure; })
        .add<bhv::action>("2", [&n] { return ++n < 5 ? bhv::status::running: bhv::status::success; })
        .add<bhv::action>("2", [&n] {  ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 2);
  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 4);
  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 5);
  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 2);
}

TEST_CASE("Running fallback node after the exception", "[fallback]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::fallback("root")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] {
            if (++n < 3)
              throw 42;
            return bhv::status::success;
        })
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE_THROWS(seq());
  REQUIRE(n == 2);
  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 4);
}
