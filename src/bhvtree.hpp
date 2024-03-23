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

private:
  virtual status tick() = 0;

  std::string_view _name;
};

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

  using base::base;

private:
  status tick() final;
};

// ?
class fallback : public control<fallback> {
public:
  using base = control<fallback>;

  using base::base;

private:
  status tick() final;
};

// =>
class parallel : public control<parallel> {
public:
  using base = control<parallel>;

  parallel(std::string_view name, size_t threshold);

private:
  status tick() final;

private:
  size_t _threshold;
};

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

template <typename Fn, typename R = return_t<Fn, status>>
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

template <typename Fn, typename R = return_t<Fn, bool>>
condition::condition(std::string_view name, Fn &&fn)
    : execution(name), _predicate(fn) {}

} // namespace bhv
} // namespace cppttl