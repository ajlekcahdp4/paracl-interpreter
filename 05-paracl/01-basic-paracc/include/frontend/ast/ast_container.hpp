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

  static i_ast_node *make_assignment_statement(variable_expression *left, i_ast_node *right, location l) {
    return new assignment_statement{left, right, l};
  }

  static i_ast_node *make_binary_expression(binary_operation op_type, i_ast_node *left, i_ast_node *right, location l) {
    return new binary_expression{op_type, left, right, l};
  }

  static i_ast_node *make_constant_expression(int val, location l) { return new constant_expression{val, l}; }

  static i_ast_node *make_error_node(std::string msg, location l) { return new error_node{msg, l}; }

  static i_ast_node *make_if_statement(i_ast_node *cond, i_ast_node *true_block, location l) {
    return new if_statement{cond, true_block, l};
  }

  static i_ast_node *make_if_statement(i_ast_node *cond, i_ast_node *true_block, i_ast_node *else_block, location l) {
    return new if_statement{cond, true_block, else_block, l};
  }

  static i_ast_node *make_print_statement(i_ast_node *expr, location l) { return new print_statement{expr, l}; }

  static i_ast_node *make_read_expression(location l) { return new read_expression{l}; }

  static i_ast_node *make_statement_block(std::vector<i_ast_node *> &&vec, location l) {
    return new statement_block{std::move(vec), l};
  }

  static i_ast_node *make_unary_expression(unary_operation op, i_ast_node *expr, location l) {
    return new unary_expression{op, expr, l};
  }

  static i_ast_node *make_variable_expression(std::string &name, location l) {
    return new variable_expression{name, l};
  }

  static i_ast_node *make_while_statement(i_ast_node *cond, i_ast_node *block, location l) {
    return new while_statement{cond, block, l};
  }
};
} // namespace paracl::frontend::ast