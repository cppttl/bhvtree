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
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cppttl {
namespace bhv {

/**
 * @brief Statuses returned by node::tick() function
 */
enum class status { running, success, failure };

/**
 * @brief Supported node types
 */
enum class node_type {
  action,
  condition,
  sequence,
  fallback,
  parallel,
  if_,
  switch_,
  invert,
  repeat,
  retry,
  force,
  custom
};

/**
 * @brief The base class of all nodes.
 */
class node {
public:
  using ptr = std::shared_ptr<node>;

  node(node_type type, std::string_view name);
  virtual ~node();
  status operator()();
  node_type type() const;
  std::string_view name() const;

private:
  virtual status tick() = 0;

  node_type _type;
  std::string_view _name;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// control nodes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Note: every execution of a control flow node with memory can be obtained
// with a non-memory BT using some auxiliary conditions

/**
 * @brief Base class for control nodes.
 * A management node can manage multiple child nodes.
 */
class basic_control : public node {
public:
  using childs_list = std::vector<node::ptr>;

  using node::node;

  childs_list const &childs() const;

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
/**
 * @brief A sequence node executes all child nodes in turn until one of them
 * fails or all of them succeed.
 */
class sequence : public control<sequence> {
public:
  using base = control<sequence>;

  sequence(std::string_view name);

private:
  status tick() final;
  void reset();

private:
  size_t _running{};
};

// ?
/**
 * @brief The fallback node executes all child nodes until one of them succeeds,
 * otherwise it fails.
 */
class fallback : public control<fallback> {
public:
  using base = control<fallback>;

  fallback(std::string_view name);

private:
  status tick() final;
  void reset();

private:
  size_t _running{};
};

// =>
/**
 * @brief The parallel node executes all the child nodes until at least M nodes
 * succeed, otherwise it fails.
 */
class parallel : public control<parallel> {
public:
  using base = control<parallel>;

  parallel(std::string_view name, size_t threshold);
  size_t threshold() const;

private:
  status tick() final;
  void reset();

private:
  using statuses = std::vector<status>;

  size_t _threshold;
  statuses _statuses;
};

// if/then/else
/**
 * @brief An if-else statement controls conditional branching.
 */
class if_ : public basic_control {
public:
  using base = basic_control;

  if_(std::string_view name);
  template <typename T, typename Node = std::decay_t<T>>
  if_(std::string_view name, T &&node);

  node::ptr condition() const;
  node::ptr then_() const;
  node::ptr else_() const;

  template <typename T, typename Node = std::decay_t<T>>
  if_ &condition(T &&node);
  template <typename Node, typename... Args> if_ &condition(Args &&...args);

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
if_::if_(std::string_view name, T &&node) : base(node_type::if_, name) {
  _childs.reserve(3);
  condition(std::forward<T>(node));
}

template <typename T, typename Node> if_ &if_::condition(T &&node) {
  size_t const idx = static_cast<size_t>(state::condition_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args> if_ &if_::condition(Args &&...args) {
  size_t const idx = static_cast<size_t>(state::condition_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

template <typename T, typename Node> if_ &if_::then_(T &&node) {
  size_t const idx = static_cast<size_t>(state::then_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args> if_ &if_::then_(Args &&...args) {
  size_t const idx = static_cast<size_t>(state::then_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

template <typename T, typename Node> if_ &if_::else_(T &&node) {
  size_t const idx = static_cast<size_t>(state::else_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<T>(node));
  return *this;
}

template <typename Node, typename... Args> if_ &if_::else_(Args &&...args) {
  size_t const idx = static_cast<size_t>(state::else_state);
  if (_childs.size() < idx + 1)
    _childs.resize(idx + 1);
  _childs[idx] = std::make_shared<Node>(std::forward<Args>(args)...);
  return *this;
}

// switch/case
/**
 * @brief The switch-case operator selects child nodes according to the given
 * predicates and executes them. For successful completion, all selected child
 * nodes must be completed with the successful status.
 */
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

  switch_(std::string_view name);

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

/**
 * @brief The base class for decorators.
 * Decorators are control nodes with single child.
 */
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

/**
 * @brief A node for inverting the child result.
 */
class invert : public decorator<invert> {
public:
  using base = decorator<invert>;

  invert(std::string_view name);

private:
  status tick() final;
};

/**
 * @brief Repeat the child node N times.
 */
class repeat : public decorator<repeat> {
public:
  using base = decorator<repeat>;

  static constexpr auto infinitely = std::numeric_limits<size_t>::max();

  repeat(std::string_view name, size_t repeat_n = infinitely);

private:
  status tick() final;
  void reset();

private:
  size_t const _n;
  size_t _i = {};
};

/**
 * @brief Retry the child node N times until it returns a successful state.
 */
class retry : public decorator<retry> {
public:
  using base = decorator<retry>;

  static constexpr auto infinitely = std::numeric_limits<size_t>::max();

  retry(std::string_view name, size_t repeat_n = infinitely);

private:
  status tick() final;
  void reset();

private:
  size_t const _n;
  size_t _i = {};
};

/**
 * @brief Force change the child status
 */
class force : public decorator<force> {
public:
  using base = decorator<force>;

  force(std::string_view name, status st);

private:
  status tick() final;

private:
  status const _status;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// execution nodes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * @brief The base class for execution nodes.
 */
class execution : public node {
public:
  using node::node;
};

template <typename Fn, typename R>
constexpr auto is_return_v = std::is_same_v<std::invoke_result_t<Fn>, R>;
template <typename Fn, typename R>
using return_t = std::enable_if_t<is_return_v<Fn, R>, R>;

/**
 * @brief An action node performs some useful task.
 * It should return the status after execution.
 * The statuses can be as follows: running, success, failure
 */
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
action::action(std::string_view name, Fn &&fn)
    : execution(node_type::action, name), _fn(fn) {}

/**
 * @brief A condition node is a predicate that is very useful for branching an
 * algorithm. It can only return success or failure statuses.
 */
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
    : execution(node_type::condition, name), _predicate(fn) {}

} // namespace bhv
} // namespace cppttl
