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

#include "ast_nodes/i_ast_node.hpp"
#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"

#include <cassert>

namespace paracl::frontend::ast {

class ast_container;

class ast_copier : public ezvis::visitor_base<i_ast_node, ast_copier, i_ast_node &> {
  using to_visit = tuple_ast_nodes;
  ast_container &m_container;

public:
  ast_copier(ast_container &container) : m_container{container} {}

  EZVIS_VISIT(to_visit);

  assignment_statement &copy(assignment_statement &);
  binary_expression    &copy(binary_expression &);
  constant_expression  &copy(constant_expression &);
  if_statement         &copy(if_statement &);
  print_statement      &copy(print_statement &);
  read_expression      &copy(read_expression &);
  statement_block      &copy(statement_block &);
  unary_expression     &copy(unary_expression &);
  variable_expression  &copy(variable_expression &);
  while_statement      &copy(while_statement &);
  error_node           &copy(error_node &);

  EZVIS_VISIT_INVOKER(copy);
};

inline i_ast_node *ast_copy(i_ast_node *node, ast_container &container) {
  if (!node) return nullptr; // In case the ast is empty. nullptr is a valid paramter
  ast_copier copier = {container};
  return &copier.apply(*node);
}

} // namespace paracl::frontend::ast

#include "ast_container.hpp"