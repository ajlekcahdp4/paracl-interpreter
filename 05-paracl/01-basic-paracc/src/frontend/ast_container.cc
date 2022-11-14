/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/ast/ast_container.hpp"

#include <memory>

namespace paracl::frontend::ast {

assignment_statement *ast_container::make_assignment_statement(variable_expression *left, i_ast_node *right,
                                                               location l) {
  m_nodes.emplace_back(std::make_unique<assignment_statement>(left, right, l));
  m_root = m_nodes.back().get();
  return static_cast<assignment_statement *>(m_nodes.back().get());
}

binary_expression *ast_container::make_binary_expression(binary_operation op_type, i_ast_node *left, i_ast_node *right,
                                                         location l) {
  m_nodes.emplace_back(std::make_unique<binary_expression>(op_type, left, right, l));
  m_root = m_nodes.back().get();
  return static_cast<binary_expression *>(m_nodes.back().get());
}

constant_expression *ast_container::make_constant_expression(int val, location l) {
  m_nodes.emplace_back(std::make_unique<constant_expression>(val, l));
  m_root = m_nodes.back().get();
  return static_cast<constant_expression *>(m_nodes.back().get());
}

error_node *ast_container::make_error_node(std::string msg, location l) {
  m_nodes.emplace_back(std::make_unique<error_node>(msg, l));
  m_root = m_nodes.back().get();
  return static_cast<error_node *>(m_nodes.back().get());
}

if_statement *ast_container::make_if_statement(i_ast_node *cond, i_ast_node *true_block, location l) {
  m_nodes.emplace_back(std::make_unique<if_statement>(cond, true_block, l));
  m_root = m_nodes.back().get();
  return static_cast<if_statement *>(m_nodes.back().get());
}

if_statement *ast_container::make_if_statement(i_ast_node *cond, i_ast_node *true_block, i_ast_node *else_block,
                                               location l) {
  m_nodes.emplace_back(std::make_unique<if_statement>(cond, true_block, else_block, l));
  m_root = m_nodes.back().get();
  return static_cast<if_statement *>(m_nodes.back().get());
}

print_statement *ast_container::make_print_statement(i_ast_node *expr, location l) {
  m_nodes.emplace_back(std::make_unique<print_statement>(expr, l));
  m_root = m_nodes.back().get();
  return static_cast<print_statement *>(m_nodes.back().get());
}

read_expression *ast_container::make_read_expression(location l) {
  m_nodes.emplace_back(std::make_unique<read_expression>(l));
  m_root = m_nodes.back().get();
  return static_cast<read_expression *>(m_nodes.back().get());
}

statement_block *ast_container::make_statement_block(std::vector<i_ast_node *> &&vec, location l) {
  m_nodes.emplace_back(std::make_unique<statement_block>(std::move(vec), l));
  m_root = m_nodes.back().get();
  return static_cast<statement_block *>(m_nodes.back().get());
}

statement_block *ast_container::make_statement_block(const std::vector<i_ast_node *> &vec, location l) {
  return make_statement_block(std::vector<i_ast_node *>{vec}, l);
}

unary_expression *ast_container::make_unary_expression(unary_operation op, i_ast_node *expr, location l) {
  m_nodes.emplace_back(std::make_unique<unary_expression>(op, expr, l));
  m_root = m_nodes.back().get();
  return static_cast<unary_expression *>(m_nodes.back().get());
}

variable_expression *ast_container::make_variable_expression(std::string &name, location l) {
  m_nodes.emplace_back(std::make_unique<variable_expression>(name, l));
  m_root = m_nodes.back().get();
  return static_cast<variable_expression *>(m_nodes.back().get());
}

while_statement *ast_container::make_while_statement(i_ast_node *cond, i_ast_node *block, location l) {
  m_nodes.emplace_back(std::make_unique<while_statement>(cond, block, l));
  m_root = m_nodes.back().get();
  return static_cast<while_statement *>(m_nodes.back().get());
}

} // namespace paracl::frontend::ast