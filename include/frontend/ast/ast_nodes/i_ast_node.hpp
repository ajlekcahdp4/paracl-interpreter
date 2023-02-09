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

#include "location.hpp"

namespace paracl::frontend::ast {

class i_ast_visitor;

class i_ast_node {
private:
  location m_loc;

public:
  i_ast_node() = default;
  i_ast_node(location l) : m_loc{l} {}

  i_ast_node(const i_ast_node &) = default;
  i_ast_node &operator=(const i_ast_node &) = default;

  virtual ~i_ast_node() {}

  virtual void accept(i_ast_visitor &) = 0;

  location loc() { return m_loc; }
};

} // namespace paracl::frontend::ast

#include "frontend/ast/visitor.hpp"

namespace paracl::frontend::ast {
template <typename t_derived> struct visitable_ast_node : public i_ast_node {
  visitable_ast_node(location l) : i_ast_node{l} {}
  virtual void accept(i_ast_visitor &visitor) override { visitor.visit(static_cast<t_derived *>(this)); }
};

} // namespace paracl::frontend::ast