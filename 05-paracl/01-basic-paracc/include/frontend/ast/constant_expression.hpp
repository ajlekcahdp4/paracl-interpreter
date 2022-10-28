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

#include "i_expression_node.hpp"

namespace paracl::frontend::ast {

class constant_expression : public i_expression_node {
public:
  int m_val;

  constant_expression(int p_val) : m_val{p_val} {}
};

static inline auto make_constant_expression(int val) { return i_expression_node_uptr{new constant_expression{val}}; }

} // namespace paracl::frontend::ast