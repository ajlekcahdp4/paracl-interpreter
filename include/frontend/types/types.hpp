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
  e_builtin,
  e_composite_function,
};

enum class builtin_type_class {
  e_builtin_int,
  e_builtin_void,
};

inline std::string builtin_type_to_string(builtin_type_class type_tag) {
  switch (type_tag) {
  case builtin_type_class::e_builtin_void: return "void";
  case builtin_type_class::e_builtin_int: return "int";
  }

  assert(0 && "Broken builtin_type_class enum");
}

class i_type;

using unique_type = std::unique_ptr<i_type>;

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

  type_builtin(builtin_type_class type_tag) : i_type{type_class::e_builtin}, m_builtin_type_tag{type_tag} {}

  bool is_equal(const i_type &rhs) const override {
    return m_type_tag == rhs.get_type() &&
        static_cast<const type_builtin &>(rhs).m_builtin_type_tag == m_builtin_type_tag;
  }

  std::string to_string() const override { return builtin_type_to_string(m_builtin_type_tag); }
  unique_type clone() const override { return std::make_unique<type_builtin>(*this); }
};

class type_composite_function : public i_type, private std::vector<unique_type> {
private:
  unique_type m_return_type;

public:
  EZVIS_VISITABLE();

  type_composite_function(std::vector<unique_type> arg_types, unique_type return_type)
      : i_type{type_class::e_composite_function}, vector{std::move(arg_types)}, m_return_type{return_type.release()} {
    assert(return_type.get() && "Return type can't be a nullptr");
  }

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
      ss << start->get()->to_string() << ")";
    }

    return ss.str();
  }

  unique_type clone() const override {
    std::vector<unique_type> args;

    for (const auto &v : *this) {
      args.push_back(v->clone());
    }

    return std::make_unique<type_composite_function>(std::move(args), m_return_type->clone());
  }

  const i_type &return_type() const & { return *m_return_type.get(); }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::empty;
  using vector::size;
};

} // namespace paracl::frontend::types