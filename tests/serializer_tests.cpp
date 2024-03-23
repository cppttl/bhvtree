#include "bhvtree.hpp"
#include "catch.hpp"
#include <bhvserializer.hpp>
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
