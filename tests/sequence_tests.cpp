#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Sequence node", "[sequence]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::sequence("root")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 3);
}

TEST_CASE("Sequence of sequences", "[sequence]") {
  int n = 0;

  // clang-format off
  auto seq0 =
      bhv::sequence("seq0")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::success; });

  auto seq1 =
      bhv::sequence("seq1")
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("4", [&n] { ++n; return bhv::status::success; });

  auto seq =
      bhv::sequence("root")
        .add(seq0)
        .add(seq1);
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 4);
}

TEST_CASE("Running sequence node", "[sequence]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::sequence("root")
        .add<bhv::action>("1", [&n] { return ++n < 2 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("2", [&n] { return ++n < 4 ? bhv::status::running : bhv::status::success; })
        .add<bhv::action>("3", [&n] { return ++n < 6 ? bhv::status::running : bhv::status::success; });
  // clang-format on

  for (int i = 0; i < 3; ++i) {
    REQUIRE(seq() == bhv::status::running);
    REQUIRE(n == 1 + i * 2);
  }

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 6);
}

TEST_CASE("Running after the fail", "[sequence]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::sequence("root")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("2", [&n] { return ++n < 3 ? bhv::status::failure : bhv::status::success; })
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; });
  // clang-format on

  REQUIRE(seq() == bhv::status::failure);
  REQUIRE(n == 2);
  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 5);
}

TEST_CASE("Running sequence node after the exception", "[sequence]") {
  int n = 0;

  // clang-format off
  auto seq =
      bhv::sequence("root")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
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
  REQUIRE(n == 5);
}
