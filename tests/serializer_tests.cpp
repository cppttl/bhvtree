#include "bhvtree.hpp"
#include "catch.hpp"
#include <bhvserializer.hpp>
#include <iostream>
#include <sstream>

using namespace cppttl;

TEST_CASE("Sequence node serialization", "[serializer]") {
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

  std::stringstream ss;
  ss << seq;
}

TEST_CASE("Fallback node serialization", "[serializer]") {
  int n = 0;

  // clang-format off
  auto fal0 =
      bhv::fallback("fal0")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::success; });

  auto seq0 =
      bhv::sequence("seq0")
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("4", [&n] { ++n; return bhv::status::success; });

  auto fal =
      bhv::fallback("root")
        .add(fal0)
        .add(seq0);
  // clang-format on

  std::stringstream ss;
  ss << fal;
}

TEST_CASE("Parallel node serialization", "[serializer]") {
  int n = 0;

  // clang-format off
  auto fal0 =
      bhv::fallback("fal0")
        .add<bhv::action>("1", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("2", [&n] { ++n; return bhv::status::success; });

  auto seq0 =
      bhv::sequence("seq0")
        .add<bhv::action>("3", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("4", [&n] { ++n; return bhv::status::success; });

  auto par1 =
      bhv::parallel("par1", 1)
        .add<bhv::action>("5", [&n] { ++n; return bhv::status::success; })
        .add<bhv::action>("6", [&n] { ++n; return bhv::status::success; });

  auto par =
      bhv::parallel("root", 2)
        .add(fal0)
        .add(seq0)
        .add(par1);
  // clang-format on

  std::stringstream ss;
  ss << par;
}

TEST_CASE("If/Then/Else node serialization", "[serializer]") {
  int n = 0;

  // clang-format off
  auto if_empty =
      bhv::if_("root");
  auto if_ =
      bhv::if_("root", bhv::condition("success", [] { return true; }));
  auto if_then =
      bhv::if_("root", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 42; return bhv::status::success; });
  auto if_else =
      bhv::if_("root", bhv::condition("success", [] { return true; }))
      .else_<bhv::action>("else", [&n]{ n = 43; return bhv::status::success; });
  auto if_then_else =
      bhv::if_("root", bhv::condition("success", [] { return true; }))
      .then_<bhv::action>("then", [&n]{ n = 42; return bhv::status::success; })
      .else_<bhv::action>("else", [&n]{ n = 43; return bhv::status::success; });
  // clang-format on

  std::stringstream ss;
  ss << if_empty;
  ss << if_;
  ss << if_then;
  ss << if_else;
  ss << if_then_else;
}

TEST_CASE("Switch/Case node serialization", "[serializer]") {
  int n, h = 0, h0 = 0, h1 = 0;

  // clang-format off
  auto switch_0 =
    bhv::switch_("my_switch")
      .case_<bhv::condition>("case 0", [&] { h0 = 0; return n == 0; })
        .handler<bhv::action>("handler 0", [&] { return ++h0 < 2 ? bhv::status::running : bhv::status::success; })
      .case_<bhv::condition>("case 1", [&] { h1 = 0; return n == 1; })
        .handler<bhv::action>("handler 1", [&] { return ++h1 < 2 ? bhv::status::running : bhv::status::failure; })
      .default_<bhv::action>("default", [&] { ++h; return bhv::status::success; });

  auto switch_1 =
    bhv::switch_("my_switch")
      .case_<bhv::condition>("case 0", [&] { h0 = 0; return n == 0; })
      .case_<bhv::condition>("case 1", [&] { h1 = 0; return n == 1; })
        .handler<bhv::action>("handler 0", [&] { return ++h0 < 2 ? bhv::status::running : bhv::status::success; })
      .case_<bhv::condition>("case 2", [&] { h1 = 0; return n == 1; })
        .handler<bhv::action>("handler 1", [&] { return ++h1 < 2 ? bhv::status::running : bhv::status::failure; })
      .default_<bhv::action>("default", [&] { ++h; return bhv::status::success; });

  auto switch_2 =
    bhv::switch_("my_switch")
       .default_<bhv::action>("default", [&] { ++h; return bhv::status::success; });
  // clang-format on

  std::stringstream ss;
  ss << switch_0;
  ss << switch_1;
  ss << switch_2;
}
