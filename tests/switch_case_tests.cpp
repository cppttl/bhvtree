#include "catch.hpp"
#include <bhvtree.hpp>

using namespace cppttl;

TEST_CASE("Switch without handlers", "[switch/case]") {
  auto switch_ = bhv::switch_("switch");

  REQUIRE(switch_() == bhv::status::failure);
}

TEST_CASE("Switch with default handler", "[switch/case]") {
  int n = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .default_<bhv::action>(
        "default handler",
          [&] {
            n = 42;
            return bhv::status::success;
          });
  // clang-format on

  REQUIRE((switch_() == bhv::status::success && n == 42));
}

TEST_CASE("Repeat default handler 3 times", "[switch/case]") {
  int n = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .default_<bhv::action>(
        "default handler",
          [&] {
            return ++n < 3
                ? bhv::status::running
                : bhv::status::success;
          });
  // clang-format on

  REQUIRE((switch_() == bhv::status::running && n == 1));
  REQUIRE((switch_() == bhv::status::running && n == 2));
  REQUIRE((switch_() == bhv::status::success && n == 3));
}

TEST_CASE("Switch with cases and without default handler", "[switch/case]") {
  int n, h;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [&n] { return n == 0; })
        .handler<bhv::action>("handler 0", [&] { h = 0; return bhv::status::success; })
      .case_<bhv::condition>("case 1", [&n] { return n == 1; })
      .case_<bhv::condition>("case 2", [&n] { return n == 2; })
      .case_<bhv::condition>("case 3", [&n] { return n == 3; })
        .handler<bhv::action>("handler 1", [&] { h = 1; return bhv::status::success; })
      .case_<bhv::condition>("case 4", [&n] { return n == 4; })
        .handler<bhv::action>("handler 2", [&] { h = 2; return bhv::status::success; });
  // clang-format on

  n = 0;
  h = -1;
  REQUIRE((switch_() == bhv::status::success && h == 0));

  for (n = 1; n < 4; ++n) {
    h = -1;
    REQUIRE((switch_() == bhv::status::success && h == 1));
  }

  n = 4;
  h = -1;
  REQUIRE((switch_() == bhv::status::success && h == 2));

  n = 5;
  h = -1;
  REQUIRE(switch_() == bhv::status::failure);
}

TEST_CASE("Switch with cases and with default handler", "[switch/case]") {
  int n, h;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [&n] { return n == 0; })
        .handler<bhv::action>("handler 0", [&] { h = 0; return bhv::status::success; })
      .case_<bhv::condition>("case 1", [&n] { return n == 1; })
        .handler<bhv::action>("handler 1", [&] { h = 1; return bhv::status::success; })
      .default_<bhv::action>("default", [&] { h = 3; return bhv::status::success; });
  // clang-format on

  n = 0;
  REQUIRE((switch_() == bhv::status::success && h == 0));
  n = 1;
  REQUIRE((switch_() == bhv::status::success && h == 1));
  n = 3;
  REQUIRE((switch_() == bhv::status::success && h == 3));
  n = 42;
  REQUIRE((switch_() == bhv::status::success && h == 3));
}

TEST_CASE("Switch with running handlers", "[switch/case]") {
  int n, h = 0, h0 = 0, h1 = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [&] { h0 = 0; return n == 0; })
        .handler<bhv::action>("handler 0", [&] { return ++h0 < 2 ? bhv::status::running : bhv::status::success; })
      .case_<bhv::condition>("case 1", [&] { h1 = 0; return n == 1; })
        .handler<bhv::action>("handler 1", [&] { return ++h1 < 2 ? bhv::status::running : bhv::status::failure; })
      .default_<bhv::action>("default", [&] { ++h; return bhv::status::success; });
  // clang-format on

  for (size_t i = 0; i < 2; ++i) {
    h = 0;

    n = 0;
    REQUIRE((switch_() == bhv::status::running && h0 == 1));
    REQUIRE((switch_() == bhv::status::success && h0 == 2));

    n = 1;
    REQUIRE((switch_() == bhv::status::running && h1 == 1));
    REQUIRE((switch_() == bhv::status::failure && h1 == 2));

    n = 3;
    REQUIRE((switch_() == bhv::status::success && h == 1));
    n = 42;
    REQUIRE((switch_() == bhv::status::success && h == 2));
  }
}

TEST_CASE("Switch with running cases and handlers", "[switch/case]") {
  int n, h = 0, c0 = 0, c1 = 0, h0 = 0, h1 = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::action>("case 0", [&] {
        if (n != 0)
          return bhv::status::failure;
        return ++c0 < 2
                ? bhv::status::running
                : bhv::status::success;
      })
        .handler<bhv::action>("handler 0", [&] {
            return ++h0 < 2
                    ? bhv::status::running
                    : bhv::status::success;
        })
      .case_<bhv::action>("case 1", [&] {
        if (n != 1)
          return bhv::status::failure;
        return ++c1 < 2
                ? bhv::status::running
                : bhv::status::success;
      })
        .handler<bhv::action>("handler 1", [&] {
            return ++h1 < 2
                    ? bhv::status::running
                    : bhv::status::failure;
        })
      .default_<bhv::action>("default", [&] {
        ++h;
        return bhv::status::success;
      });
  // clang-format on

  for (size_t i = 0; i < 2; ++i) {
    h = 0;
    h0 = h1 = 0;
    c0 = c1 = 0;

    n = 0;
    REQUIRE((switch_() == bhv::status::running && c0 == 1 && h0 == 0 &&
             c1 == 0 && h1 == 0));
    REQUIRE((switch_() == bhv::status::running && c0 == 2 && h0 == 1 &&
             c1 == 0 && h1 == 0));
    REQUIRE((switch_() == bhv::status::success && c0 == 2 && h0 == 2 &&
             c1 == 0 && h1 == 0));

    n = 1;
    REQUIRE((switch_() == bhv::status::running && c1 == 1 && h1 == 0));
    REQUIRE((switch_() == bhv::status::running && c1 == 2 && h1 == 1));
    REQUIRE((switch_() == bhv::status::failure && c1 == 2 && h1 == 2));

    n = 3;
    REQUIRE((switch_() == bhv::status::success && h == 1));
    n = 42;
    REQUIRE((switch_() == bhv::status::success && h == 2));
  }
}

TEST_CASE("Switch with different handlers for one case", "[switch/case]") {
  int n, h0 = 0, h1 = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [&n] { return n == 0; })
        .handler<bhv::action>("handler 0", [&] { return ++h0 < 2 ? bhv::status::running : bhv::status::success; })
      .case_<bhv::condition>("case 1", [&n] { return n == 0; })
        .handler<bhv::action>("handler 1", [&] { return ++h1 < 3 ? bhv::status::running : bhv::status::failure; });
  // clang-format on

  n = 0;
  REQUIRE((switch_() == bhv::status::running && h0 == 1 && h1 == 1));
  REQUIRE((switch_() == bhv::status::running && h0 == 2 && h1 == 2));
  REQUIRE((switch_() == bhv::status::failure && h0 == 2 && h1 == 3));
}

TEST_CASE("Switch recovery after the exception", "[switch/case]") {
  int n, h0 = 0, h1 = 0;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [&] { return n == 0; })
        .handler<bhv::action>("handler 0", [&] {
          if (++h0 < 2)
            throw 42;
          return bhv::status::success;
        })
      .case_<bhv::condition>("case 1", [&] { return n == 1; })
        .handler<bhv::action>("handler 1", [&] {
          if (++h1 < 2)
            throw 43;
          return bhv::status::failure;
        });
  // clang-format on

  for (size_t i = 0; i < 2; ++i) {
    h0 = 0, h1 = 0;

    n = 0;
    REQUIRE_THROWS(switch_());
    REQUIRE(h0 == 1);
    REQUIRE((switch_() == bhv::status::success && h0 == 2));

    n = 1;
    REQUIRE_THROWS(switch_());
    REQUIRE(h1 == 1);
    REQUIRE((switch_() == bhv::status::failure && h1 == 2));
  }
}
