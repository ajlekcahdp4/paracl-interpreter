/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/ast/ast_copier.hpp"
#include "frontend/ast/ast_nodes/error_node.hpp"
#include "frontend/ast/ast_nodes/if_statement.hpp"
#include "frontend/ast/ast_nodes/unary_expression.hpp"
#include "frontend/ast/ast_nodes/variable_expression.hpp"
#include "frontend/ast/visitor.hpp"

#include <memory>

namespace paracl::frontend::ast {

void ast_copier::visit(assignment_statement *ptr) {
  auto ptr_copy = m_container.emplace_back<assignment_statement>(*ptr);

  ast_node_visit(*this, ptr->left());
  ptr_copy->left() = get_return_as<variable_expression>();
  ast_node_visit(*this, ptr->right());
  ptr_copy->right() = get_return();

  return_node(ptr_copy);
}

void ast_copier::visit(binary_expression *ptr) {
  auto ptr_copy = m_container.emplace_back<binary_expression>(*ptr);

  ast_node_visit(*this, ptr->left());
  ptr_copy->left() = get_return();
  ast_node_visit(*this, ptr->right());
  ptr_copy->right() = get_return();

  return_node(ptr_copy);
}

void ast_copier::visit(print_statement *ptr) {
  auto ptr_copy = m_container.emplace_back<print_statement>(*ptr);

  ast_node_visit(*this, ptr->expr());
  ptr_copy->expr() = get_return();

  return_node(ptr_copy);
}

void ast_copier::visit(read_expression *ptr) {
  auto ptr_copy = m_container.emplace_back<read_expression>(*ptr);
  return_node(ptr_copy);
}

void ast_copier::visit(constant_expression *ptr) {
  auto ptr_copy = m_container.emplace_back<constant_expression>(*ptr);
  return_node(ptr_copy);
}

void ast_copier::visit(if_statement *ptr) {
  auto ptr_copy = m_container.emplace_back<if_statement>(*ptr);

  ast_node_visit(*this, ptr->cond());
  ptr_copy->cond() = get_return();
  ast_node_visit(*this, ptr->true_block());
  ptr_copy->true_block() = get_return();

  if (ptr_copy->else_block()) {
    ast_node_visit(*this, ptr->else_block());
    ptr_copy->else_block() = get_return();
  }

  return_node(ptr_copy);
}

void ast_copier::visit(statement_block *ptr) {
  auto ptr_copy = m_container.emplace_back<statement_block>(*ptr);

  for (auto &v : ptr_copy->statements()) {
    ast_node_visit(*this, v);
    v = get_return();
  }

  return_node(ptr_copy);
}

void ast_copier::visit(unary_expression *ptr) {
  auto ptr_copy = m_container.emplace_back<unary_expression>(*ptr);
  ast_node_visit(*this, ptr->child());
  ptr_copy->child() = get_return();
  return_node(ptr_copy);
}

void ast_copier::visit(variable_expression *ptr) {
  auto ptr_copy = m_container.emplace_back<variable_expression>(*ptr);
  return_node(ptr_copy);
}

void ast_copier::visit(while_statement *ptr) {
  auto ptr_copy = m_container.emplace_back<while_statement>(*ptr);

  ast_node_visit(*this, ptr->cond());
  ptr_copy->cond() = get_return();
  ast_node_visit(*this, ptr->block());
  ptr_copy->block() = get_return();

  return_node(ptr_copy);
}

void ast_copier::visit(error_node *ptr) {
  auto ptr_copy = m_container.emplace_back<error_node>(*ptr);
  return_node(ptr_copy);
}

i_ast_node *ast_copy(i_ast_node *node, ast_container &container) {
  ast_copier copier = {container};
  ast_node_visit(copier, node);
  return copier.get_return();
}

} // namespace paracl::frontend::ast