/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ast.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend::ast {

i_ast_node *assignment_statement::clone() {
  return ast::make_assignment_statement(static_cast<variable_expression *>(m_left->clone()), m_right->clone(), loc());
}

i_ast_node *binary_expression::clone() {
  return ast::make_binary_expression(m_operation_type, m_left->clone(), m_right->clone(), loc());
}

i_ast_node *constant_expression::clone() { return ast::make_constant_expression(m_val, loc()); }

i_ast_node *error_node::clone() { return ast::make_error_node(m_error_message, loc()); }

i_ast_node *if_statement::clone() {
  if (m_else_block) {
    return ast::make_if_statement(m_condition->clone(), m_true_block->clone(), m_else_block->clone(), loc());
  }
  return ast::make_if_statement(m_condition->clone(), m_true_block->clone(), loc());
}

i_ast_node *print_statement::clone() { return ast::make_print_statement(m_expr->clone(), loc()); }

i_ast_node *read_expression::clone() { return ast::make_read_expression(loc()); }

i_ast_node *statement_block::clone() {
  std::vector<i_ast_node *> vec_copy;
  for (const auto &v : m_statements) {}
}

i_ast_node *unary_expression::clone() {}

i_ast_node *variable_expression::clone() {}

i_ast_node *while_statement::clone() {}

} // namespace paracl::frontend::ast