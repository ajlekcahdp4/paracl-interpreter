/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/copier.hpp"

namespace paracl::frontend::ast {

void ast_copier::visit(assignment_statement *) {}

void ast_copier::visit(binary_expression *) {}

void ast_copier::visit(constant_expression *) {}

void ast_copier::visit(if_statement *) {}

void ast_copier::visit(print_statement *) {}

void ast_copier::visit(read_expression *) {}

void ast_copier::visit(statement_block *) {}

void ast_copier::visit(unary_expression *) {}

void ast_copier::visit(variable_expression *) {}

void ast_copier::visit(while_statement *) {}

void ast_copier::visit(error_node *) {}

void ast_copy(i_ast_node *node, ast_container &container) {
  ast_copier copier(container);
  ast_node_visit(copier, node);
}

} // namespace paracl::frontend::ast