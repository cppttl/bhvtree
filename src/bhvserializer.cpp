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

#include "bhvserializer.hpp"
#include "bhvtree.hpp"
#include <stdexcept>
#include <string>

namespace cppttl {
namespace bhv {
namespace {

using namespace std::string_literals;

const char *type_name(node const &n) { return to_string(n.type()); }

std::string mask(std::string const &str) {
  auto it = str.find('"');
  if (it == std::string::npos)
    return str;

  std::string res;
  res.reserve(str.size() + 8);

  std::string::size_type p = 0;

  for (; it != std::string::npos; it = str.find('"', it)) {
    res += str.substr(p, it - p);
    res += '\\';
    p = it;
    it += 1;
  }

  res += str.substr(p);

  return res;
}

std::string node_name(node const &n) {
  return "\""s + mask(std::string(n.name())) + "\""s;
}

class serializer final {
public:
  serializer(std::ostream &stream);

  serializer &operator<<(node const &ref);

private:
  std::ostream &indent(size_t layer) const;
  void save(node const &ref, size_t layer) const;
  void save(action const &ref, size_t layer) const;
  void save(condition const &ref, size_t layer) const;
  void save(sequence const &ref, size_t layer) const;
  void save(fallback const &ref, size_t layer) const;
  void save(parallel const &ref, size_t layer) const;
  void save(if_ const &ref, size_t layer) const;
  void save(switch_ const &ref, size_t layer) const;
  void save(invert const &ref, size_t layer) const;
  void save(repeat const &ref, size_t layer) const;
  void save(retry const &ref, size_t layer) const;
  void save(force const &ref, size_t layer) const;

private:
  std::ostream &_stream;
};

serializer::serializer(std::ostream &stream) : _stream(stream) {}

serializer &serializer::operator<<(node const &ref) {
  save(ref, 0);
  return *this;
}

std::ostream &serializer::indent(size_t layer) const {
  static const char *spaces = "  ";
  for (size_t i = 0; i < layer; ++i) {
    _stream << spaces;
  }
  return _stream;
}

void serializer::save(node const &ref, size_t layer) const {
  switch (ref.type()) {
  case node_type::action:
    save(static_cast<action const &>(ref), layer);
    break;
  case node_type::condition:
    save(static_cast<condition const &>(ref), layer);
    break;
  case node_type::sequence:
    save(static_cast<sequence const &>(ref), layer);
    break;
  case node_type::fallback:
    save(static_cast<fallback const &>(ref), layer);
    break;
  case node_type::parallel:
    save(static_cast<parallel const &>(ref), layer);
    break;
  case node_type::if_:
    save(static_cast<if_ const &>(ref), layer);
    break;
  case node_type::switch_:
    save(static_cast<switch_ const &>(ref), layer);
    break;
  case node_type::invert:
    save(static_cast<invert const &>(ref), layer);
    break;
  case node_type::repeat:
    save(static_cast<repeat const &>(ref), layer);
    break;
  case node_type::retry:
    save(static_cast<retry const &>(ref), layer);
    break;
  case node_type::force:
    save(static_cast<force const &>(ref), layer);
    break;
  case node_type::custom:
    throw std::runtime_error("Unsupported node type");
    break;
  }
}

void serializer::save(action const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref) << std::endl;
}

void serializer::save(condition const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref) << std::endl;
}

void serializer::save(sequence const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref);

  if (!ref.childs().empty()) {
    _stream << " {" << std::endl;
    for (auto &child : ref.childs())
      save(*child, layer + 1);
    indent(layer) << "}";
  }

  _stream << std::endl;
}

void serializer::save(fallback const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref);

  if (!ref.childs().empty()) {
    _stream << " {" << std::endl;
    for (auto &child : ref.childs())
      save(*child, layer + 1);
    indent(layer) << "}";
  }

  _stream << std::endl;
}

void serializer::save(parallel const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " threshold=" << ref.threshold() << " "
                << node_name(ref);

  if (!ref.childs().empty()) {
    _stream << " {" << std::endl;
    for (auto &child : ref.childs())
      save(*child, layer + 1);
    indent(layer) << "}";
  }

  _stream << std::endl;
}

void serializer::save(if_ const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref);

  if (!ref.childs().empty()) {
    _stream << " {" << std::endl;

    if (ref.condition()) {
      indent(layer + 1) << "condition" << std::endl;
      save(*ref.condition(), layer + 2);
    }
    if (ref.then_()) {
      indent(layer + 1) << "then" << std::endl;
      save(*ref.then_(), layer + 2);
    }
    if (ref.else_()) {
      indent(layer + 1) << "else" << std::endl;
      save(*ref.else_(), layer + 2);
    }

    indent(layer) << "}";
  }

  _stream << std::endl;
}

void serializer::save(switch_ const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref);

  if (!ref.childs().empty() || ref.default_handler()) {
    _stream << " {" << std::endl;

    node::cptr handler;

    for (auto &&case_ : ref) {
      if (handler && handler != case_.handler())
        save(*handler, layer + 1);
      save(*case_.condition(), layer);
      handler = case_.handler();
    }

    if (handler)
      save(*handler, layer + 1);

    if (ref.default_handler()) {
      indent(layer) << "default" << std::endl;
      save(*ref.default_handler(), layer + 1);
    }

    indent(layer) << "}";
  }

  _stream << std::endl;
}

void serializer::save(invert const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " " << node_name(ref) << std::endl;

  if (!ref.childs().empty()) {
    for (auto &child : ref.childs())
      save(*child, layer + 1);
  }
}

void serializer::save(repeat const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " n=" << ref.count() << " "
                << node_name(ref) << std::endl;

  if (!ref.childs().empty()) {
    for (auto &child : ref.childs())
      save(*child, layer + 1);
  }
}

void serializer::save(retry const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " n=" << ref.count() << " "
                << node_name(ref) << std::endl;

  if (!ref.childs().empty()) {
    for (auto &child : ref.childs())
      save(*child, layer + 1);
  }
}

void serializer::save(force const &ref, size_t layer) const {
  indent(layer) << type_name(ref) << " status=" << to_string(ref.result())
                << " " << node_name(ref) << std::endl;

  if (!ref.childs().empty()) {
    for (auto &child : ref.childs())
      save(*child, layer + 1);
  }
}

} // namespace

std::ostream &operator<<(std::ostream &stream, node const &ref) {
  serializer ss(stream);
  ss << ref;
  return stream;
}

} // namespace bhv
} // namespace cppttl
