#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Sequence node", "[control]") {
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

TEST_CASE("Sequence of sequences", "[control]") {
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
