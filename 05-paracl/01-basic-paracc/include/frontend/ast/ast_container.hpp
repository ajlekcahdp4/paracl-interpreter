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

#include "ast_copier.hpp"
#include "ast_nodes.hpp"

#include <memory>
#include <vector>

namespace paracl::frontend::ast {

using i_ast_node_uptr = std::unique_ptr<i_ast_node>;

class ast_container {
private:
  std::vector<i_ast_node_uptr> m_nodes;

  i_ast_node *m_root = nullptr;

  friend class ast_copier;

public:
  ast_container() = default;

  void        set_root_ptr(i_ast_node *ptr) { m_root = ptr; }
  i_ast_node *get_root_ptr() { return m_root; }

  assignment_statement *make_assignment_statement(variable_expression *, i_ast_node *, location);
  binary_expression    *make_binary_expression(binary_operation, i_ast_node *, i_ast_node *, location);
  constant_expression  *make_constant_expression(int, location);
  error_node           *make_error_node(std::string, location);

  if_statement *make_if_statement(i_ast_node *, i_ast_node *, location);
  if_statement *make_if_statement(i_ast_node *, i_ast_node *, i_ast_node *, location);

  print_statement *make_print_statement(i_ast_node *, location);
  read_expression *make_read_expression(location);

  statement_block *make_statement_block(std::vector<i_ast_node *> &&, location);
  statement_block *make_statement_block(const std::vector<i_ast_node *> &, location);

  unary_expression    *make_unary_expression(unary_operation, i_ast_node *, location);
  variable_expression *make_variable_expression(std::string &, location);
  while_statement     *make_while_statement(i_ast_node *, i_ast_node *, location);
};

} // namespace paracl::frontend::ast

#include "ast_copier.hpp"