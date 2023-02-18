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

template <typename T> T &trivial_ast_node_copy(const T &ref, ast_container &cont) { return *cont.make_node<T>(ref); }

read_expression     &ast_copier::copy(read_expression &ref) { return trivial_ast_node_copy(ref, m_container); }
variable_expression &ast_copier::copy(variable_expression &ref) { return trivial_ast_node_copy(ref, m_container); }
error_node          &ast_copier::copy(error_node &ref) { return trivial_ast_node_copy(ref, m_container); }
constant_expression &ast_copier::copy(constant_expression &ref) { return trivial_ast_node_copy(ref, m_container); }

binary_expression &ast_copier::copy(binary_expression &ref) {
  return *m_container.make_node<binary_expression>(ref.op_type(), apply(ref.left()), apply(ref.right()), ref.loc());
}

print_statement &ast_copier::copy(print_statement &ref) {
  return *m_container.make_node<print_statement>(apply(ref.expr()), ref.loc());
}

unary_expression &ast_copier::copy(unary_expression &ref) {
  return *m_container.make_node<unary_expression>(ref.op_type(), apply(ref.expr()), ref.loc());
}

while_statement &ast_copier::copy(while_statement &ref) {
  return *m_container.make_node<while_statement>(apply(ref.cond()), apply(ref.block()), ref.loc());
}

assignment_statement &ast_copier::copy(assignment_statement &ref) {
  auto copy = m_container.make_node<assignment_statement>(*ref.rbegin(), ref.right(), ref.loc());

  for (auto start = std::next(ref.rbegin()), finish = ref.rend(); start != finish; ++start) {
    copy->append_variable(*start);
  }

  return *copy;
}

if_statement &ast_copier::copy(if_statement &ref) {
  if (ref.else_block()) {
    return *m_container.make_node<if_statement>(apply(ref.cond()), apply(ref.true_block()), apply(*ref.else_block()),
                                                ref.loc());
  }

  return *m_container.make_node<if_statement>(apply(ref.cond()), apply(ref.true_block()), ref.loc());
}

statement_block &ast_copier::copy(statement_block &ref) {
  auto copy = m_container.make_node<statement_block>();

  for (const auto &v : ref) {
    copy->append_statement(apply(*v));
  }

  return *copy;
}

} // namespace paracl::frontend::ast