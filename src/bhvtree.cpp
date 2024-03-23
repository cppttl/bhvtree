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
#include <stdexcept>

namespace cppttl {
namespace bhv {

// node
node::node(std::string_view name) : _name(name) {}

node::~node() {}

status node::operator()() { return tick(); }

std::string_view node::name() const { return _name; }

// sequence
status sequence::tick() {
  status st = status::success;

  try {
    for (; _running < _childs.size(); ++_running) {
      auto &child = _childs[_running];
      st = (*child)();
      if (st != status::success)
        break;
    }

    if (st != status::running)
      reset();
  } catch (...) {
    reset();
    throw;
  }

  return st;
}

void sequence::reset() { _running = 0; }

// fallback
status fallback::tick() {
  status st = status::failure;

  try {
    for (; _running < _childs.size(); ++_running) {
      auto &child = _childs[_running];
      st = (*child)();
      if (st != status::failure)
        break;
    }

    if (st != status::running)
      reset();
  } catch (...) {
    reset();
    throw;
  }

  return st;
}

void fallback::reset() { _running = 0; }

// parallel
parallel::parallel(std::string_view name, size_t threshold)
    : base(name), _threshold(threshold) {}

status parallel::tick() {
  status st = status::success;

  try {
    _statuses.resize(_childs.size(), status::running);

    size_t success = {};
    size_t failed = {};

    for (size_t i = 0; i < _childs.size(); ++i) {
      auto &child = _childs[i];
      auto &st = _statuses[i];

      if (st == status::running)
        st = (*child)();
      if (st == status::success)
        ++success;
      else if (st == status::failure)
        ++failed;
    }

    st = success >= _threshold                  ? status::success
         : failed > _childs.size() - _threshold ? status::failure
                                                : status::running;

    if (st != status::running)
      reset();
  } catch (...) {
    reset();
    throw;
  }

  return st;
}

void parallel::reset() { _statuses.clear(); }

// invert
status invert::tick() {
  if (_childs.empty() || !_childs.front())
    throw std::runtime_error(
        "There is no controllable node under the inverter node");

  auto status = (*_childs.front())();

  switch (status) {
  case status::success:
    return status::failure;
  case status::failure:
    return status::success;
  case status::running:
    return status::running;
  }

  throw std::runtime_error("The child node returned an unknown status");

  return status::failure;
}

// action
status action::tick() { return _fn(); }

// condition
status condition::tick() {
  return _predicate() ? status::success : status::failure;
}

// if_
status if_::tick() {
  if (_childs.empty() || !_childs[static_cast<size_t>(state::condition_state)])
    throw std::runtime_error("There is no condition node under the 'if' node");

  status st = status::failure;

  try {
    do {
      if (!_childs[static_cast<size_t>(_state)]) {
        _state = state::condition_state;
        return status::failure;
      }

      st = (*_childs[static_cast<size_t>(_state)])();

      switch (st) {
      case status::running:
        return st;
      case status::success: {
        _state = _state == state::condition_state ? state::then_state
                                                  : state::break_state;
        break;
      }
      case status::failure: {
        _state = _state == state::condition_state ? state::else_state
                                                  : state::break_state;
        break;
      }
      }
    } while (_state != state::break_state);

    _state = state::condition_state;
  } catch (...) {
    _state = state::condition_state;
    throw;
  }
  return st;
}

// switch_
case_proxy::case_proxy(switch_ &stmt) : _switch(stmt) {}

status switch_::tick() {
  if (_childs.size() != _map.size())
    throw std::runtime_error("The switch expression is in an invalid state. "
                             "Some cases are incorrectly mapped to handlers.");

  status st = status::failure;

  // try {
  //   do {
  //     if (!_childs[static_cast<size_t>(_state)]) {
  //       _state = state::condition_state;
  //       return status::failure;
  //     }

  //     st = (*_childs[static_cast<size_t>(_state)])();

  //     switch (st) {
  //     case status::running:
  //       return st;
  //     case status::success: {
  //       _state = _state == state::condition_state ? state::then_state
  //                                                 : state::break_state;
  //       break;
  //     }
  //     case status::failure: {
  //       _state = _state == state::condition_state ? state::else_state
  //                                                 : state::break_state;
  //       break;
  //     }
  //     }
  //   } while (_state != state::break_state);

  //   _state = state::condition_state;
  // } catch (...) {
  //   _state = state::condition_state;
  //   throw;
  // }

  do {
    switch (_state) {
    case state::collect_state: {
      _state = state::default_state;
      break;
    }

    case state::default_state: {
      if (_default_handler) {
        st = (*_default_handler)();
        if (st == status::running)
          return st;
      } else {
        st = status::failure;
      }
      _state = state::break_state;
      break;
    }

    default:
      throw std::runtime_error("Unknown state of swith/case handler");
    }
  } while (_state != state::break_state);

  _state = state::collect_state;

  return st;
}

} // namespace bhv
} // namespace cppttl
