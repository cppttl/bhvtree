#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Parallel node", "[parallel]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::parallel("root", 2)
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("4", [&n] { ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 4);
}

TEST_CASE("Parallel failed node", "[parallel]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::parallel("root", 2)
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::failure; })
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("4", [&n] { ++n; return bhv::status::failure; });
  // clang-format on

  REQUIRE(seq() == bhv::status::failure);
  REQUIRE(n == 4);
}

TEST_CASE("Running a parallel node with successful completion", "[parallel]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::parallel("root", 2)
        .add<bhv::action>("1", [&n] { n = 1; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { return ++n < 3 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("3", [&n] { return ++n < 4 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("4", [&n] { return ++n < 5 ? bhv::status::running : bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 4);
  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 7);
  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 4);
}

TEST_CASE("Running a parallel node with failure completion", "[parallel]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::parallel("root", 2)
        .add<bhv::action>("1", [&n] { n = 1; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { return ++n < 3 ? bhv::status::running : bhv::status::failure; })
        .add<bhv::action>("3", [&n] { return ++n < 4 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("4", [&n] { return ++n < 5 ? bhv::status::running : bhv::status::failure; });
  // clang-format on

  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 4);
  REQUIRE(seq() == bhv::status::failure);
  REQUIRE(n == 7);
  REQUIRE(seq() == bhv::status::running);
  REQUIRE(n == 4);
}

TEST_CASE("Running a parallel after the exception", "[parallel]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::parallel("root", 2)
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::failure; })
        .add<bhv::action>("2", [&n] { return ++n < 3 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("3", [&n] {
            if (++n < 4)
                throw 42;
            return n < 4 ? bhv::status::running : bhv::status::success;
        })
        .add<bhv::action>("4", [&n] { return ++n < 5 ? bhv::status::running : bhv::status::success; });
  // clang-format on

  REQUIRE_THROWS(seq());
  REQUIRE(n == 3);
  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 7);
}
