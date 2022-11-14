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

#include "ast/ast_container.hpp"
#include "ast/visitor.hpp"
#include <iostream>

namespace paracl::frontend::ast {

class ast_copier : public i_ast_visitor {
private:
  ast_container &m_container;

public:
  ast_copier(ast_container &container) : m_container{container} {}

  void visit(assignment_statement *) override;
  void visit(binary_expression *) override;
  void visit(constant_expression *) override;
  void visit(if_statement *) override;
  void visit(print_statement *) override;
  void visit(read_expression *) override;
  void visit(statement_block *) override;
  void visit(unary_expression *) override;
  void visit(variable_expression *) override;
  void visit(while_statement *) override;
  void visit(error_node *) override;
};

void ast_copy(i_ast_node *node, ast_container &container);

} // namespace paracl::frontend::ast