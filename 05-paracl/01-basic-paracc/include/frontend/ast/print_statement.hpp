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

class print_statement : public i_statement_node {
  i_expression_node_uptr m_expr;

public:
  print_statement(i_expression_node_uptr &&p_expr) : m_expr{std::move(p_expr)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

static inline i_statement_node_uptr make_print_statement(i_expression_node_uptr &&expr) {
  return std::make_unique<print_statement>(std::move(expr));
}

} // namespace paracl::frontend::ast