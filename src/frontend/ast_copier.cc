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

assignment_statement &ast_copier_new::copy(assignment_statement &ref) {
  auto copy = m_container.emplace_back<assignment_statement>(*ref.rbegin(), ref.right(), ref.loc());
  for (auto start = std::next(ref.rbegin()), finish = ref.rend(); start != finish; ++start) {
    copy->append_variable(*start);
  }
  return *copy;
}

binary_expression &ast_copier_new::copy(binary_expression &ref) {
  return *m_container.emplace_back<binary_expression>(ref.op_type(), &apply(*ref.left()), &apply(*ref.right()),
                                                      ref.loc());
}

read_expression &ast_copier_new::copy(read_expression &ref) { return *m_container.emplace_back<read_expression>(ref); }

print_statement &ast_copier_new::copy(print_statement &ref) {
  return *m_container.emplace_back<print_statement>(&apply(*ref.expr()), ref.loc());
}

constant_expression &ast_copier_new::copy(constant_expression &ref) {
  return *m_container.emplace_back<constant_expression>(ref);
}

if_statement &ast_copier_new::copy(if_statement &ref) {
  if (ref.else_block()) {
    return *m_container.emplace_back<if_statement>(&apply(*ref.cond()), &apply(*ref.true_block()),
                                                   &apply(*ref.else_block()), ref.loc());
  }

  return *m_container.emplace_back<if_statement>(&apply(*ref.cond()), &apply(*ref.true_block()), ref.loc());
}

statement_block &ast_copier_new::copy(statement_block &ref) {
  auto copy = m_container.emplace_back<statement_block>();

  for (const auto &v : ref) {
    copy->append_statement(&apply(*v));
  }

  return *copy;
}

unary_expression &ast_copier_new::copy(unary_expression &ref) {
  return *m_container.emplace_back<unary_expression>(ref.op_type(), &apply(*ref.expr()), ref.loc());
}

variable_expression &ast_copier_new::copy(variable_expression &ref) {
  return *m_container.emplace_back<variable_expression>(ref);
}

while_statement &ast_copier_new::copy(while_statement &ref) {
  return *m_container.emplace_back<while_statement>(&apply(*ref.cond()), &apply(*ref.block()), ref.loc());
}

error_node &ast_copier_new::copy(error_node &ref) { return *m_container.emplace_back<error_node>(ref); }

i_ast_node *ast_copy(i_ast_node *node, ast_container &container) {
  assert(node);
  if (!node) return nullptr;
  ast_copier_new copier = {container};
  return &copier.apply(*node);
}

} // namespace paracl::frontend::ast