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
#include <memory>

namespace paracl::frontend::ast {

class i_ast_visitor;

class i_ast_node {
private:
  location m_loc;

public:
  i_ast_node() = default;
  i_ast_node(location l) : m_loc{l} {}

  virtual ~i_ast_node() {}
  virtual void accept(i_ast_visitor &) = 0;

  location loc() { return m_loc; }
};

using i_ast_node_uptr = std::unique_ptr<i_ast_node>;

} // namespace paracl::frontend::ast

#include "frontend/ast/visitor.hpp"