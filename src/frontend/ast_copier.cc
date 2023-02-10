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
#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/visitor.hpp"

#include <cassert>
#include <memory>

namespace paracl::frontend::ast {

void ast_copier::visit(assignment_statement *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<assignment_statement>(copy_subtree(ptr->left()), copy_subtree(ptr->right()),
                                                             ptr->loc()));
}

void ast_copier::visit(binary_expression *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<binary_expression>(ptr->op_type(), copy_subtree(ptr->left()),
                                                          copy_subtree(ptr->right()), ptr->loc()));
}

void ast_copier::visit(read_expression *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<read_expression>(*ptr));
}

void ast_copier::visit(print_statement *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<print_statement>(copy_subtree(ptr->expr()), ptr->loc()));
}

void ast_copier::visit(constant_expression *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<constant_expression>(*ptr));
}

void ast_copier::visit(if_statement *ptr) {
  assert(ptr);

  if (ptr->else_block()) {
    return_node(m_container.emplace_back<if_statement>(copy_subtree(ptr->cond()), copy_subtree(ptr->true_block()),
                                                       copy_subtree(ptr->else_block()), ptr->loc()));
  } else {
    return_node(
        m_container.emplace_back<if_statement>(copy_subtree(ptr->cond()), copy_subtree(ptr->true_block()), ptr->loc()));
  }
}

void ast_copier::visit(statement_block *ptr) {
  assert(ptr);
  auto copy = m_container.emplace_back<statement_block>();

  for (const auto &v : *ptr) {
    copy->append_statement(copy_subtree(v));
  }

  return_node(copy);
}

void ast_copier::visit(unary_expression *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<unary_expression>(ptr->op_type(), copy_subtree(ptr->expr()), ptr->loc()));
}

void ast_copier::visit(variable_expression *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<variable_expression>(*ptr));
}

void ast_copier::visit(while_statement *ptr) {
  assert(ptr);
  return_node(
      m_container.emplace_back<while_statement>(copy_subtree(ptr->cond()), copy_subtree(ptr->block()), ptr->loc()));
}

void ast_copier::visit(error_node *ptr) {
  assert(ptr);
  return_node(m_container.emplace_back<error_node>(*ptr));
}

i_ast_node *ast_copy(i_ast_node *node, ast_container &container) {
  assert(node);
  if (!node) return nullptr;
  ast_copier copier = {container};
  ast_node_visit(copier, node);
  return copier.get_return();
}

} // namespace paracl::frontend::ast