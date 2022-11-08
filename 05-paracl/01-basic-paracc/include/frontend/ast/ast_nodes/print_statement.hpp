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

class print_statement : public i_ast_node {
  i_ast_node_uptr m_expr;

public:
  print_statement(i_ast_node_uptr &&p_expr, location l) : i_ast_node{l}, m_expr{std::move(p_expr)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  i_ast_node *expr() { return m_expr.get(); }
};

static inline i_ast_node_uptr make_print_statement(i_ast_node_uptr &&expr, location l) {
  return std::make_unique<print_statement>(std::move(expr), l);
}

} // namespace paracl::frontend::ast