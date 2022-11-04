/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/ast/visitor/resolver.hpp"

#include "frontend/ast/unary_expression.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace paracl::utils::serialization;

namespace paracl::frontend::ast {

void ast_resolve_visitor::visit(assignment_statement *ptr) {
  ast_node_visit(*this, ptr->right());
  ast_node_visit(*this, ptr->left());
}

void ast_resolve_visitor::visit(binary_expression *ptr) {}

void ast_resolve_visitor::visit(constant_expression *ptr) {}

void ast_resolve_visitor::visit(if_statement *ptr) {}

void ast_resolve_visitor::visit(print_statement *ptr) {}

void ast_resolve_visitor::visit(read_expression *ptr) {}

void ast_resolve_visitor::visit(statement_block *ptr) {
  m_symtab.begin_scope();
  for (auto &statement : ptr->m_statements)
    ast_node_visit(*this, statement.get());
  m_symtab.end_scope();
}

void ast_resolve_visitor::visit(unary_expression *ptr) {}

void ast_resolve_visitor::visit(variable_expression *ptr) { if (!m_symtab.declared_in_scope()) }

void ast_resolve_visitor::visit(while_statement *ptr) {}

void ast_resolve(i_ast_node *node) {
  ast_resolve_visitor resolver{};
  ast_node_visit(resolver, node);
}

} // namespace paracl::frontend::ast