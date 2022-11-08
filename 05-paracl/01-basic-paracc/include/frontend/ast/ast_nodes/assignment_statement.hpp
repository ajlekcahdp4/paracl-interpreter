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
#include "variable_expression.hpp"

namespace paracl::frontend::ast {

class assignment_statement : public i_ast_node {
  variable_expression_uptr m_left;
  i_ast_node_uptr          m_right;

public:
  assignment_statement(variable_expression_uptr &&left, i_ast_node_uptr &&right, location l)
      : i_ast_node{l}, m_left{std::move(left)}, m_right{std::move(right)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  variable_expression *left() { return m_left.get(); }
  i_ast_node          *right() { return m_right.get(); }
};

static inline i_ast_node_uptr make_assignment_statement(variable_expression_uptr &&left, i_ast_node_uptr &&right,
                                                        location l) {
  return std::make_unique<assignment_statement>(std::move(left), std::move(right), l);
}

} // namespace paracl::frontend::ast