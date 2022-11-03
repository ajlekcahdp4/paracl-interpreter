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

#include "i_ast_node.hpp"

namespace paracl::frontend::ast {

class constant_expression : public i_expression_node {
  int m_val;

public:
  constant_expression(int p_val) : m_val{p_val} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }
  int  value() const { return m_val; }
};

static inline i_expression_node_uptr make_constant_expression(int val) {
  return std::make_unique<constant_expression>(val);
}

} // namespace paracl::frontend::ast