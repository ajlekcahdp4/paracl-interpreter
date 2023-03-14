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

class return_statement final : public i_expression {
  i_expression *m_expr;

  EZVIS_VISITABLE();

public:
  return_statement(i_expression *p_expr, location l) : i_expression{l}, m_expr{p_expr} {}

  bool empty() const { return !m_expr; }
  i_expression &expr() const {
    if (m_expr == nullptr) throw std::runtime_error{"Attempt to dereference empty return statement"};
    return *m_expr;
  }
};

} // namespace paracl::frontend::ast