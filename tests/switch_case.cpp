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
  int n = 0;
  int h = -1;

  // clang-format off
  auto switch_ =
    bhv::switch_("switch")
      .case_<bhv::condition>("case 0", [n] { return n == 0; })
        .handler<bhv::action>("handler 0", [&] { h = 0; return bhv::status::success; })
      .case_<bhv::condition>("case 1", [n] { return n == 1; })
      .case_<bhv::condition>("case 2", [n] { return n == 2; })
      .case_<bhv::condition>("case 3", [n] { return n == 3; })
        .handler<bhv::action>("handler 1", [&] { h = 1; return bhv::status::success; })
      .case_<bhv::condition>("case 4", [n] { return n == 4; })
        .handler<bhv::action>("handler 2", [&] { h = 2; return bhv::status::success; });
  // clang-format on

  // TODO

  // n = 0;
  // h = -1;
  // REQUIRE((switch_() == bhv::status::success && h == 0));

  // for (n = 1; n < 4; ++n) {
  //   h = -1;
  //   REQUIRE((switch_() == bhv::status::success && h == 1));
  // }

  // n = 4;
  // h = -1;
  // REQUIRE((switch_() == bhv::status::success && h == 2));

  // n = 5;
  // h = -1;
  // REQUIRE(switch_() == bhv::status::failure);
}

// TEST_CASE("Switch with cases and with default handler", "[switch/case]") {
//   int n = 0;

//   // clang-format off
//   auto switch_ =
//     bhv::switch_("switch")
//       .default_<bhv::action>(
//         "default handler",
//           [&] {
//             n = 42;
//             return bhv::status::success;
//           });
//   // clang-format on

//   REQUIRE((switch_() == bhv::status::success && n == 42));
// }
