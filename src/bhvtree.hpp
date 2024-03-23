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

#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cppttl {
namespace bhv {

enum class status { running, success, failure };

class node {
public:
  using ptr = std::shared_ptr<node>;

  node(std::string_view name);
  virtual ~node();
  status operator()();
  std::string_view name() const;

private:
  virtual status tick() = 0;

  std::string_view _name;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// control nodes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Note: every execution of a control flow node with memory can be obtained
// with a non-memory BT using some auxiliary conditions

// Sequence, Fallback, Parallel, and Decorator
class basic_control : public node {
public:
  using childs_list = std::vector<node::ptr>;

  using node::node;

protected:
  childs_list _childs;
};

template <typename Impl> class control : public basic_control {
public:
  using basic_control::basic_control;

  template <typename Node, typename... Args> Impl &add(Args &&...args);
  template <typename T, typename Node = std::decay_t<T>> Impl &add(T &&node);
};

template <typename Impl>
template <typename Node, typename... Args>
Impl &control<Impl>::add(Args &&...args) {
  _childs.emplace_back(std::make_shared<Node>(std::forward<Args>(args)...));
  return static_cast<Impl &>(*this);
}

template <typename Impl>
template <typename T, typename Node>
Impl &control<Impl>::add(T &&node) {
  _childs.emplace_back(std::make_shared<Node>(std::forward<T>(node)));
  return static_cast<Impl &>(*this);
}

// ->
class sequence : public control<sequence> {
public:
  using base = control<sequence>;

  using base::control;

private:
  status tick() final;
  void reset();

private:
  size_t _running{};
};

// ?
class fallback : public control<fallback> {
public:
  using base = control<fallback>;

  using base::control;

private:
  status tick() final;
  void reset();

private:
  size_t _running{};
};

// =>
class parallel : public control<parallel> {
public:
  using base = control<parallel>;

  parallel(std::string_view name, size_t threshold);

private:
  status tick() final;
  void reset();

private:
  using statuses = std::vector<status>;

  size_t _threshold;
  statuses _statuses;
};

// if/then/else
class if_ : public basic_control {
public:
  using base = basic_control;

  template <typename T, typename Node = std::decay_t<T>>
  if_(std::string_view name, T &&node);

  template <typename T, typename Node = std::decay_t<T>> if_ &then_(T &&node);
  template <typename Node, typename... Args> if_ &then_(Args &&...args);

  template <typename T, typename Node = std::decay_t<T>> if_ &else_(T &&node);
  template <typename Node, typename... Args> if_ &else_(Args &&...args);

private:
  status tick() final;
  void reset();

private:
  enum class state : size_t {
    condition_state,
    then_state,
    else_state,
    break_state
  };

  state _state = state::condition_state;
};

template <typename T, typename Node>
if_::if_(std::string_view name, T &&node) : base(name) {
  _childs.resize(3);
  _childs[0] = std::make_shared<Node>(std::forward<T>(node));
}

template <typename T, typename Node> if_ &if_::then_(T &&node) {
  _childs[1] = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args> if_ &if_::then_(Args &&...args) {
  _childs[1] = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

template <typename T, typename Node> if_ &if_::else_(T &&node) {
  _childs[2] = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args> if_ &if_::else_(Args &&...args) {
  _childs[2] = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

// switch/case
class switch_;

class case_proxy {
private:
  case_proxy(switch_ &stmt);

public:
  case_proxy(case_proxy const &) = default;
  case_proxy(case_proxy &&) = default;

  template <typename Cond, typename Condition = std::decay_t<Cond>>
  case_proxy case_(Cond &&condition) &&;
  template <typename Cond, typename... Args>
  case_proxy case_(Args &&...args) &&;

  template <typename T, typename Node = std::decay_t<T>>
  switch_ &handler(T &&node) &&;
  template <typename Node, typename... Args>
  switch_ &handler(Args &&...args) &&;

private:
  switch_ &_switch;

  friend class switch_;
};

class switch_ : public basic_control {
public:
  using base = basic_control;

  using base::base;

  template <typename C> case_proxy case_(C &&condition);
  template <typename C, typename... Args> case_proxy case_(Args &&...args);

  template <typename T, typename Node = std::decay_t<T>>
  switch_ &default_(T &&node);
  template <typename Node, typename... Args> switch_ &default_(Args &&...args);

private:
  status tick() final;
  void reset();
  status match();
  status exec();

private:
  enum class state : size_t { match, exec };

  using handlers_map = std::vector<size_t>;
  using statuses = std::vector<status>;
  using handler_statuses = std::vector<std::pair<size_t, status>>;

  state _state = state::match;
  statuses _match_statuses;
  handler_statuses _handler_statuses;

  childs_list _handlers;
  node::ptr _default_handler;
  handlers_map _map;

  friend class case_proxy;
};

template <typename C> case_proxy switch_::case_(C &&condition) {
  return case_proxy(*this).case_<C>(std::forward<C>(condition));
}

template <typename C, typename... Args>
case_proxy switch_::case_(Args &&...args) {
  return case_proxy(*this).case_<C, Args...>(std::forward<Args>(args)...);
}

template <typename T, typename Node> switch_ &switch_::default_(T &&node) {
  _default_handler = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args>
switch_ &switch_::default_(Args &&...args) {
  _default_handler = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

template <typename Cond, typename Condition>
case_proxy case_proxy::case_(Cond &&condition) && {
  _switch._map.emplace_back(_switch._handlers.size());
  try {
    _switch._childs.emplace_back(
        std::make_shared<Condition>(std::forward<Cond>(condition)));
  } catch (...) {
    _switch._map.pop_back();
    throw;
  }
  return *this;
}

template <typename C, typename... Args>
case_proxy case_proxy::case_(Args &&...args) && {
  _switch._map.emplace_back(_switch._handlers.size());
  try {
    _switch._childs.emplace_back(
        std::make_shared<C>(std::forward<Args>(args)...));
  } catch (...) {
    _switch._map.pop_back();
    throw;
  }
  return *this;
}

template <typename T, typename Node> switch_ &case_proxy::handler(T &&node) && {
  _switch._handlers.emplace_back(std::make_shared<Node>(std::forward<T>(node)));
  return _switch;
}

template <typename Node, typename... Args>
switch_ &case_proxy::handler(Args &&...args) && {
  _switch._handlers.emplace_back(
      std::make_shared<Node>(std::forward<Args>(args)...));
  return _switch;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// decorators
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <typename Impl> class decorator : public basic_control {
public:
  using basic_control::basic_control;

  template <typename Node, typename... Args> Impl &child(Args &&...args);
  template <typename T, typename Node = std::decay_t<T>> Impl &child(T &&node);
};

template <typename Impl>
template <typename Node, typename... Args>
Impl &decorator<Impl>::child(Args &&...args) {
  _childs.resize(1);
  node::ptr child = std::make_shared<Node>(std::forward<Args>(args)...);
  std::swap(_childs.front(), child);
  return static_cast<Impl &>(*this);
}

template <typename Impl>
template <typename T, typename Node>
Impl &decorator<Impl>::child(T &&node) {
  _childs.resize(1);
  node::ptr child = std::make_shared<Node>(std::forward<T>(node));
  std::swap(_childs.front(), child);
  return static_cast<Impl &>(*this);
}

// !
class invert : public decorator<invert> {
public:
  using base = decorator<invert>;

  using base::decorator;

private:
  status tick() final;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// execution nodes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Action and Condition
class execution : public node {
public:
  using node::node;
};

template <typename Fn, typename R>
constexpr auto is_return_v = std::is_same_v<std::invoke_result_t<Fn>, R>;
template <typename Fn, typename R>
using return_t = std::enable_if_t<is_return_v<Fn, R>, R>;

class action : public execution {
public:
  using handler = std::function<status()>;

  template <typename Fn, typename R = return_t<Fn, status>>
  action(std::string_view name, Fn &&fn);

private:
  status tick() final;

  handler _fn;
};

template <typename Fn, typename R>
action::action(std::string_view name, Fn &&fn) : execution(name), _fn(fn) {}

class condition : public execution {
public:
  using predicate = std::function<bool()>;

  template <typename Fn, typename R = return_t<Fn, bool>>
  condition(std::string_view name, Fn &&fn);

private:
  status tick() final;

  predicate _predicate;
};

template <typename Fn, typename R>
condition::condition(std::string_view name, Fn &&fn)
    : execution(name), _predicate(fn) {}

} // namespace bhv
} // namespace cppttl
