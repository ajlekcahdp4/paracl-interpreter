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
  variable_expression *m_left;
  i_ast_node          *m_right;

public:
  assignment_statement(variable_expression *left, i_ast_node *right, location l)
      : i_ast_node{l}, m_left{left}, m_right{right} {}

  void accept(i_ast_visitor &visitor) override { visitor.visit(this); }

  variable_expression *left() { return m_left; }
  i_ast_node          *right() { return m_right; }
};

} // namespace paracl::frontend::ast