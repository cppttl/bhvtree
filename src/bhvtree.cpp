/*
 Copyright (c) 2024 Vladislav Volkov <cppttl@gmail.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "bhvtree.hpp"

namespace cppttl {
namespace bhv {

// node
node::node(std::string_view name) : _name(name) {}

node::~node() {}

status node::operator()() { return tick(); }

// sequence
status sequence::tick() {
  for (auto &&child : _childs) {
    auto st = (*child)();
    if (st == status::running)
      return status::running;
    else if (st == status::failure)
      return status::failure;
  }
  return status::success;
}

// fallback
status fallback::tick() {
  for (auto &&child : _childs) {
    auto st = (*child)();
    if (st == status::running)
      return status::running;
    else if (st == status::success)
      return status::success;
  }
  return status::failure;
}

// parallel
parallel::parallel(std::string_view name, size_t threshold)
    : base(name), _threshold(threshold) {}

status parallel::tick() {
  size_t success = {};
  size_t failed = {};

  for (auto &&child : _childs) {
    auto st = (*child)();
    if (st == status::success)
      ++success;
    else if (st == status::failure)
      ++failed;
  }

  return success >= _threshold                  ? status::success
         : failed > _childs.size() - _threshold ? status::failure
                                                : status::running;
}

// action
status action::tick() { return _fn(); }

// condition
status condition::tick() {
  return _predicate() ? status::success : status::failure;
}

} // namespace bhv
} // namespace cppttl
