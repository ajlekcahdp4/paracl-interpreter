/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/ast/ast_nodes/assignment_statement.hpp"
#include "frontend/ast/ast_nodes/binary_expression.hpp"
#include "frontend/ast/ast_nodes/constant_expression.hpp"
#include "frontend/ast/ast_nodes/error_node.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/ast/ast_nodes/if_statement.hpp"
#include "frontend/ast/ast_nodes/print_statement.hpp"
#include "frontend/ast/ast_nodes/read_expression.hpp"
#include "frontend/ast/ast_nodes/statement_block.hpp"
#include "frontend/ast/ast_nodes/unary_expression.hpp"
#include "frontend/ast/ast_nodes/variable_expression.hpp"
#include "frontend/ast/ast_nodes/while_statement.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend::ast {

i_ast_node_uptr assignment_statement::clone() {
  auto copy_left = variable_expression_uptr{static_cast<variable_expression *>(m_left->clone().release())};
  return make_assignment_statement(std::move(copy_left), m_right->clone(), loc());
}

i_ast_node_uptr binary_expression::clone() {
  return make_binary_expression(m_operation_type, m_left->clone(), m_right->clone(), loc());
}

i_ast_node_uptr constant_expression::clone() { return make_constant_expression(m_val, loc()); }
i_ast_node_uptr error_node::clone() { return make_error_node(m_error_message, loc()); }

i_ast_node_uptr if_statement::clone() {
  if (m_else_block.get()) {
    return make_if_statement(m_condition->clone(), m_true_block->clone(), m_else_block->clone(), loc());
  }
  return make_if_statement(m_condition->clone(), m_true_block->clone(), loc());
}

i_ast_node_uptr print_statement::clone() { return make_print_statement(m_expr->clone(), loc()); }

i_ast_node_uptr read_expression::clone() { return make_read_expression(loc()); }

i_ast_node_uptr statement_block::clone() {
  std::vector<i_ast_node_uptr> vec_copy;

  for (const auto &v : m_statements) {}
}

i_ast_node_uptr unary_expression::clone() {}

i_ast_node_uptr variable_expression::clone() {}

i_ast_node_uptr while_statement::clone() {}

} // namespace paracl::frontend::ast