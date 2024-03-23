#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("If without handlers", "[if/then/else]") {
  // clang-format off
  auto if_success =
        bhv::if_("if", bhv::condition("success", [] { return true; }));
  auto if_failure =
        bhv::if_("if", bhv::condition("failure", [] { return false; }));
  auto if_running =
        bhv::if_("if", bhv::action("running", [] { return bhv::status::running; }));
  // clang-format on

  REQUIRE(if_success() == bhv::status::failure);
  REQUIRE(if_failure() == bhv::status::failure);
  REQUIRE(if_running() == bhv::status::running);
}

TEST_CASE("If then", "[if/then/else]") {
  int n = 0;

  // clang-format off
  auto if_success =
      bhv::if_("if", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 42; return bhv::status::success; });
  auto if_failure =
      bhv::if_("if", bhv::condition("failure", [] { return false; }))
      .then_<bhv::action>("then", [&n]{ n = 43; return bhv::status::success; });
  auto if_running =
      bhv::if_("if", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 44; return bhv::status::running; });
  // clang-format on

  REQUIRE((if_success() == bhv::status::success && n == 42));
  REQUIRE((if_failure() == bhv::status::failure && n == 42));
  REQUIRE((if_running() == bhv::status::running && n == 44));
}

TEST_CASE("If else", "[if/then/else]") {
  int n = 0;

  // clang-format off
  auto if_success =
      bhv::if_("if", bhv::condition("success", [] { return true; }))
      .else_<bhv::action>("else", [&n]{ n = 42; return bhv::status::success; });
  auto if_failure =
      bhv::if_("if", bhv::condition("failure", [] { return false; }))
      .else_<bhv::action>("else", [&n]{ n = 43; return bhv::status::success; });
  auto if_running =
      bhv::if_("if", bhv::condition("failure", [] { return false; }))
      .else_<bhv::action>("else", [&n]{ n = 44; return bhv::status::running; });
  // clang-format on

  REQUIRE((if_success() == bhv::status::failure && n == 0));
  REQUIRE((if_failure() == bhv::status::success && n == 43));
  REQUIRE((if_running() == bhv::status::running && n == 44));
}

TEST_CASE("If then else", "[if/then/else]") {
  int n = 0;

  // clang-format off
  auto if_success =
      bhv::if_("if", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 42; return bhv::status::success; })
      .else_<bhv::action>("else", [&n]{ n = 43; return bhv::status::success; });
  auto if_failure =
      bhv::if_("if", bhv::condition("failure", [] { return false; }))
      .then_<bhv::action>("then", [&n]{ n = 45; return bhv::status::success; })
      .else_<bhv::action>("else", [&n]{ n = 46; return bhv::status::success; });
  auto if_running_then =
      bhv::if_("if", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 47; return bhv::status::running; })
      .else_<bhv::action>("else", [&n]{ n = 48; return bhv::status::running; });
  auto if_running_else =
      bhv::if_("if", bhv::condition("success", [] { return false; }))
      .then_<bhv::action>("then", [&n]{ n = 49; return bhv::status::running; })
      .else_<bhv::action>("else", [&n]{ n = 50; return bhv::status::running; });
  // clang-format on

  REQUIRE((if_success() == bhv::status::success && n == 42));
  REQUIRE((if_failure() == bhv::status::success && n == 46));
  REQUIRE((if_running_then() == bhv::status::running && n == 47));
  REQUIRE((if_running_else() == bhv::status::running && n == 50));
}

TEST_CASE("If then else recovery after the exception", "[if/then/else]") {
  int n = 0;
  bool condition_executed = false;

  // clang-format off
  auto if_success_throw =
      bhv::if_("if", bhv::condition("success", [&] { condition_executed = true; return true; }))
      .then_<bhv::action>("then", [&n]{ n = 42; throw n; return bhv::status::success; })
      .else_<bhv::action>("else", [&n]{ n = 43; throw n; return bhv::status::success; });
  auto if_failure_throw =
      bhv::if_("if", bhv::condition("failure", [&] { condition_executed = true; return false; }))
      .then_<bhv::action>("then", [&n]{ n = 45; throw n; return bhv::status::success; })
      .else_<bhv::action>("else", [&n]{ n = 46; throw n; return bhv::status::success; });
  auto if_running_then_throw =
      bhv::if_("if", bhv::condition("success", [&] { condition_executed = true; return true; }))
      .then_<bhv::action>("then", [&n]{ n = 47; throw n; return bhv::status::running; })
      .else_<bhv::action>("else", [&n]{ n = 48; throw n; return bhv::status::running; });
  auto if_running_else_throw =
      bhv::if_("if", bhv::condition("success", [&] { condition_executed = true; return false; }))
      .then_<bhv::action>("then", [&n]{ n = 49; throw n; return bhv::status::running; })
      .else_<bhv::action>("else", [&n]{ n = 50; throw n; return bhv::status::running; });
  // clang-format on

  for (int i = 0; i < 2; ++i) {
    condition_executed = false;
    REQUIRE_THROWS(if_success_throw());
    REQUIRE(condition_executed);
  }

  for (int i = 0; i < 2; ++i) {
    condition_executed = false;
    REQUIRE_THROWS(if_failure_throw());
    REQUIRE(condition_executed);
  }

  for (int i = 0; i < 2; ++i) {
    condition_executed = false;
    REQUIRE_THROWS(if_running_then_throw());
    REQUIRE(condition_executed);
  }

  for (int i = 0; i < 2; ++i) {
    condition_executed = false;
    REQUIRE_THROWS(if_running_else_throw());
    REQUIRE(condition_executed);
  }
}
