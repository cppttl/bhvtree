#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Invert success status", "[inverter]") {
  int n = 0;

  // clang-format off
  auto inv =
        bhv::invert("inv")
        .child<bhv::condition>("a1", [] { return true; });
  // clang-format on

  REQUIRE(inv() == bhv::status::failure);
}

TEST_CASE("Invert failure status", "[inverter]") {
  int n = 0;

  // clang-format off
  auto inv =
        bhv::invert("inv")
        .child<bhv::condition>("a1", [] { return false; });
  // clang-format on

  REQUIRE(inv() == bhv::status::success);
}

TEST_CASE("Invert running status", "[inverter]") {
  int n = 0;

  // clang-format off
  auto inv =
        bhv::invert("inv")
        .child<bhv::action>("a1", [] { return bhv::status::running; });
  // clang-format on

  REQUIRE(inv() == bhv::status::running);
}

TEST_CASE("Invert no child", "[inverter]") {
  int n = 0;

  // clang-format off
  auto inv =
        bhv::invert("inv");
  // clang-format on

  REQUIRE_THROWS(inv());
}

TEST_CASE("Invert invlaid status", "[inverter]") {
  int n = 0;

  static size_t const invalid_status = 42;
  static bhv::status const *pstatus =
      reinterpret_cast<bhv::status const *>(&invalid_status);

  // clang-format off
  auto inv =
        bhv::invert("inv")
        .child<bhv::action>("a1", [&] { return *pstatus; });
  // clang-format on

  REQUIRE_THROWS(inv());
}

TEST_CASE("Fallback to inverted node", "[inverter]") {
  int n = 0;

  // clang-format off
  auto a =
      bhv::fallback("a")
        .add(bhv::invert("inv")
             .child<bhv::condition>("a1", [] { return true; }))
        .add<bhv::action>("a2", [&n]  { n = 42; return bhv::status::success; });

  auto b =
      bhv::fallback("b")
        .add(bhv::invert("inv")
             .child<bhv::condition>("b1", [] { return false; }))
        .add<bhv::action>("b2", [&n]  { n = 43; return bhv::status::success; });

  auto seq =
      bhv::sequence("root")
        .add(a)
        .add(b);
  // clang-format on

  REQUIRE(seq() == bhv::status::success);
  REQUIRE(n == 42);
}
