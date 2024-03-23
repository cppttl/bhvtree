#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Condition node", "[condition]") {
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
