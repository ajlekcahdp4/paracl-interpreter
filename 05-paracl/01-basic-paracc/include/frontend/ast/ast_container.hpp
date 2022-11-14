/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ast_nodes.hpp"
#include <vector>

namespace paracl::frontend::ast {

struct ast_container {
  std::vector<i_ast_node_uptr> m_nodes;

  assignment_statement *make_assignment_statement(variable_expression *left, i_ast_node *right, location l);

  binary_expression *make_binary_expression(binary_operation op_type, i_ast_node *left, i_ast_node *right, location l);

  constant_expression *make_constant_expression(int val, location l);

  error_node *make_error_node(std::string msg, location l);

  if_statement *make_if_statement(i_ast_node *cond, i_ast_node *true_block, location l);

  if_statement *make_if_statement(i_ast_node *cond, i_ast_node *true_block, i_ast_node *else_block, location l);

  print_statement *make_print_statement(i_ast_node *expr, location l);

  read_expression *make_read_expression(location l);

  statement_block *make_statement_block(std::vector<i_ast_node *> &&vec, location l);

  unary_expression *make_unary_expression(unary_operation op, i_ast_node *expr, location l);

  variable_expression *make_variable_expression(std::string &name, location l);

  while_statement *make_while_statement(i_ast_node *cond, i_ast_node *block, location l);
};
} // namespace paracl::frontend::ast