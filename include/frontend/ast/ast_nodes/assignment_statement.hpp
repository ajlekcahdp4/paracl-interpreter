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

struct assignment_statement final : public visitable_ast_node<assignment_statement> {
  variable_expression *m_left;
  i_ast_node          *m_right;

  assignment_statement(variable_expression *left, i_ast_node *right, location l)
      : visitable_ast_node{l}, m_left{left}, m_right{right} {}
};

} // namespace paracl::frontend::ast