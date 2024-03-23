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

TEST_CASE("Fallback node", "[control]") {
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

TEST_CASE("Failed fallback node", "[control]") {
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

TEST_CASE("Parallel node", "[control]") {
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

TEST_CASE("Parallel fallback node", "[control]") {
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

TEST_CASE("Condition node", "[control]") {
  int n = 0;

  // clang-format off
  auto a =
      bhv::fallback("a")
        .add<bhv::condition>("a1", [] { return true; })
        .add<bhv::action>("a2", [&n]  { n = 42; return bhv::status::success; });

  auto b =
      bhv::fallback("b")
        .add<bhv::condition>("b1", [] { return false; })
        .add<bhv::action>("b2", [&n]  { n = 43; return bhv::status::success; });

  auto seq =
      bhv::sequence("root")
        .add(a)
        .add(b);
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 43);
}
