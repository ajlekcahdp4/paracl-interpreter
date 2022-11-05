/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/semantic_analyzer.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace paracl::utils::serialization;

namespace paracl::frontend::ast {

void semantic_analyzer_visitor::visit(assignment_statement *ptr) {
  ast_node_visit(*this, ptr->right());
  ast_node_visit(*this, ptr->left());
}

void semantic_analyzer_visitor::visit(binary_expression *ptr) {
  ast_node_visit(*this, ptr->right());
  ast_node_visit(*this, ptr->left());
}

void semantic_analyzer_visitor::visit(constant_expression *ptr) {}

void semantic_analyzer_visitor::visit(print_statement *ptr) {}

void semantic_analyzer_visitor::visit(read_expression *ptr) {}

void semantic_analyzer_visitor::visit(error_node *ptr) {}

void semantic_analyzer_visitor::visit(statement_block *ptr) {
  m_scopes.begin_scope(&ptr->m_symtab);
  for (auto &statement : ptr->m_statements)
    ast_node_visit(*this, statement.get());
  m_scopes.end_scope();
}

void semantic_analyzer_visitor::visit(if_statement *ptr) {
  m_scopes.begin_scope(ptr->true_symtab());
  ast_node_visit(*this, ptr->true_block());
  m_scopes.end_scope();
  if (ptr->else_block() != nullptr) {
    m_scopes.begin_scope(ptr->else_symtab());
    ast_node_visit(*this, ptr->else_block());
    m_scopes.end_scope();
  }
}

void semantic_analyzer_visitor::visit(while_statement *ptr) {
  m_scopes.begin_scope(ptr->symbol_table());
  ast_node_visit(*this, ptr->block());
  m_scopes.end_scope();
}

void semantic_analyzer_visitor::visit(unary_expression *ptr) { ast_node_visit(*this, ptr->child()); }

void semantic_analyzer_visitor::visit(variable_expression *ptr) { m_scopes.declare(std::string{ptr->name()}); }

void ast_resolve(i_ast_node *node) {
  semantic_analyzer_visitor resolver{};
  ast_node_visit(resolver, node);
}

} // namespace paracl::frontend::ast