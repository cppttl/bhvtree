#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Force change status without child", "[force]") {
  // clang-format off
  auto force =
        bhv::force("force", bhv::status::success);
  // clang-format on

  REQUIRE_THROWS(force());
}

TEST_CASE("Force change the success status", "[force]") {
  // clang-format off
  auto force_failure_1 =
        bhv::force("force", bhv::status::failure)
        .child<bhv::condition>("a1", [] { return true; });
  auto force_failure_2 =
        bhv::force("force", bhv::status::failure)
        .child<bhv::condition>("a1", [] { return false; });
  auto force_failure_3 =
        bhv::force("force", bhv::status::failure)
        .child<bhv::action>("a1", [] { return bhv::status::running; });
  // clang-format on

  REQUIRE(force_failure_1() == bhv::status::failure);
  REQUIRE(force_failure_2() == bhv::status::failure);
  REQUIRE(force_failure_3() == bhv::status::running);
}

TEST_CASE("Force change the failure status", "[force]") {
  // clang-format off
  auto force_success_1 =
        bhv::force("force", bhv::status::success)
        .child<bhv::condition>("a1", [] { return true; });
  auto force_success_2 =
        bhv::force("force", bhv::status::success)
        .child<bhv::condition>("a1", [] { return false; });
  auto force_success_3 =
        bhv::force("force", bhv::status::success)
        .child<bhv::action>("a1", [] { return bhv::status::running; });
  // clang-format on

  REQUIRE(force_success_1() == bhv::status::success);
  REQUIRE(force_success_2() == bhv::status::success);
  REQUIRE(force_success_3() == bhv::status::running);
}
