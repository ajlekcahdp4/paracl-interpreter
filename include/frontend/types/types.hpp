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

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

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

  type_class get_class() const { return m_type_tag; }

  virtual ~i_type() {}
};

class type {
private:
  std::unique_ptr<i_type> m_impl = nullptr;

private:
  void check_impl() const {
    if (!m_impl) throw std::runtime_error{"Bad type dereference"};
  }

  type(std::unique_ptr<i_type> ptr) : m_impl{std::move(ptr)} {}

public:
  type() = default;

  template <typename T, typename... Ts> static type make(Ts &&...args) {
    return type{std::unique_ptr<T>{new T{std::forward<Ts>(args)...}}};
  }

  type(type &&) = default;
  type &operator=(type &&) = default;

  type(const type &rhs) : m_impl{rhs ? rhs.m_impl->clone() : nullptr} {}
  type &operator=(const type &rhs) {
    if (this == &rhs) return *this;
    type temp{rhs};
    swap(temp);
    return *this;
  }

  ~type() = default;

public:
  i_type &base() & {
    check_impl();
    return *m_impl;
  }

  const i_type &base() const & {
    check_impl();
    return *m_impl;
  }

  friend bool operator==(const type &lhs, const type &rhs) { return lhs.base().is_equal(rhs.base()); }
  friend bool operator!=(const type &lhs, const type &rhs) { return !(lhs == rhs); }

  friend bool operator==(const type &lhs, const i_type &rhs) { return lhs.base().is_equal(rhs); }
  friend bool operator==(const i_type &lhs, const type &rhs) { return rhs.base().is_equal(lhs); }
  friend bool operator!=(const type &lhs, const i_type &rhs) { return !(lhs == rhs); }
  friend bool operator!=(const i_type &lhs, const type &rhs) { return !(lhs == rhs); }

  std::string to_string() const { return base().to_string(); }
  void swap(type &rhs) { std::swap(*this, rhs); }
  operator bool() const { return m_impl.get(); }
};

class type_builtin final : public i_type {
private:
  builtin_type_class m_builtin_type_tag;

  EZVIS_VISITABLE();

public:
  static const type &type_int() {
    static const type obj = type::make<type_builtin>(builtin_type_class::E_BUILTIN_INT);
    return obj;
  }

  static const type &type_void() {
    static const type obj = type::make<type_builtin>(builtin_type_class::E_BUILTIN_VOID);
    return obj;
  }

public:
  type_builtin(builtin_type_class type_tag) : i_type{type_class::E_BUILTIN}, m_builtin_type_tag{type_tag} {}

  bool is_equal(const i_type &rhs) const override {
    return m_type_tag == rhs.get_class() &&
        static_cast<const type_builtin &>(rhs).m_builtin_type_tag == m_builtin_type_tag;
  }

  std::string to_string() const override { return builtin_type_to_string(m_builtin_type_tag); }
  unique_type clone() const override { return std::make_unique<type_builtin>(*this); }
};

class type_composite_function : public i_type, private std::vector<type> {
public:
  type m_return_type;

  EZVIS_VISITABLE();

public:
  type_composite_function(std::vector<type> arg_types, type return_type)
      : i_type{type_class::E_COMPOSITE_FUNCTION}, vector{std::move(arg_types)}, m_return_type{return_type} {}

  bool is_equal(const i_type &rhs) const override {
    if (m_type_tag != rhs.get_class()) return false;
    const auto &cast_rhs = static_cast<const type_composite_function &>(rhs);
    return (
        m_return_type == cast_rhs.m_return_type && vector::size() == cast_rhs.size() &&
        std::equal(cbegin(), cend(), cast_rhs.cbegin())
    );
  }

  void set_argument_types(const std::vector<type> &arg_types) {
    vector::clear();
    for (auto &&arg : arg_types) {
      vector::push_back(arg);
    }
  }

  std::string to_string() const override {
    std::vector<std::string> arg_types_str;

    for (const auto &v : *this) {
      arg_types_str.push_back(v.to_string());
    }

    return fmt::format(
        "({}) func({})", m_return_type ? m_return_type.to_string() : "undetermined", fmt::join(arg_types_str, ", ")
    );
    ;
  }

  unique_type clone() const override {
    std::vector<type> args;

    for (const auto &v : *this) {
      args.push_back(v);
    }

    return std::make_unique<type_composite_function>(std::move(args), m_return_type);
  }

  type &return_type() & { return m_return_type; }
  const type &return_type() const & { return m_return_type; }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::empty;
  using vector::size;
};

} // namespace paracl::frontend::types