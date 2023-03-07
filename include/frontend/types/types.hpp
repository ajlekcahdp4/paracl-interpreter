/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ezvis/ezvis.hpp"
#include "location.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

namespace paracl::frontend::types {

enum class type_class {
  E_BUILTIN,
  E_COMPOSITE_FUNCTION,
};

enum class builtin_type_class {
  E_BUILTIN_INT,
  E_BUILTIN_VOID,
};

inline std::string builtin_type_to_string(builtin_type_class type_tag) {
  switch (type_tag) {
  case builtin_type_class::E_BUILTIN_VOID: return "void";
  case builtin_type_class::E_BUILTIN_INT: return "int";
  }

  assert(0 && "Broken builtin_type_class enum");
}

class i_type;

using unique_type = std::unique_ptr<i_type>;
using shared_type = std::shared_ptr<i_type>;

class i_type : public ezvis::visitable_base<i_type> {
protected:
  type_class m_type_tag;

protected:
  i_type(type_class type_tag) : m_type_tag{type_tag} {}

public:
  EZVIS_VISITABLE();

  virtual unique_type clone() const = 0;
  virtual std::string to_string() const = 0;
  virtual bool is_equal(const i_type &) const = 0;

  type_class get_type() const { return m_type_tag; }

  virtual ~i_type() {}
};

class type_builtin final : public i_type {
private:
  builtin_type_class m_builtin_type_tag;

public:
  EZVIS_VISITABLE();

  type_builtin(builtin_type_class type_tag) : i_type{type_class::E_BUILTIN}, m_builtin_type_tag{type_tag} {}

  bool is_equal(const i_type &rhs) const override {
    return m_type_tag == rhs.get_type() &&
        static_cast<const type_builtin &>(rhs).m_builtin_type_tag == m_builtin_type_tag;
  }

  std::string to_string() const override { return builtin_type_to_string(m_builtin_type_tag); }
  unique_type clone() const override { return std::make_unique<type_builtin>(*this); }
};

class type_composite_function : public i_type, private std::vector<shared_type> {
public:
  shared_type m_return_type;

public:
  EZVIS_VISITABLE();

  type_composite_function(std::vector<shared_type> arg_types, shared_type return_type)
      : i_type{type_class::E_COMPOSITE_FUNCTION}, vector{std::move(arg_types)}, m_return_type{return_type} {}

  bool is_equal(const i_type &rhs) const override {
    if (m_type_tag != rhs.get_type()) return false;
    const auto &cast_rhs = static_cast<const type_composite_function &>(rhs);
    return (
        m_return_type->is_equal(*cast_rhs.m_return_type) && vector::size() == cast_rhs.size() &&
        std::equal(cbegin(), cend(), cast_rhs.cbegin())
    );
  }

  std::string to_string() const override {
    std::stringstream ss;
    ss << "(" << m_return_type->to_string() << ")"
       << " func(";

    if (!vector::empty()) {
      auto start = vector::begin();
      for (auto finish = std::prev(vector::end()); start != finish; ++start) {
        ss << start->get()->to_string() << ", ";
      }
      ss << start->get()->to_string();
    }
    ss << ")";

    return ss.str();
  }

  unique_type clone() const override {
    std::vector<shared_type> args;

    for (const auto &v : *this) {
      args.push_back(v->clone());
    }

    return std::make_unique<type_composite_function>(std::move(args), m_return_type->clone());
  }

  shared_type return_type() & { return m_return_type; }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::empty;
  using vector::size;
};

using shared_func_type = std::shared_ptr<type_composite_function>;

struct builtin_types {
  types::shared_type m_void = std::make_shared<types::type_builtin>(types::builtin_type_class::E_BUILTIN_VOID);
  types::shared_type m_int = std::make_shared<types::type_builtin>(types::builtin_type_class::E_BUILTIN_INT);
};

} // namespace paracl::frontend::types