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
#include "frontend/ast/ast_copier.hpp"

#include <memory>

namespace paracl::frontend::ast {

ast_container::ast_container(const ast_container &other) {
  ast_container temp = {};
  auto          root_ptr = ast_copy(other.get_root_ptr(), temp);
  temp.set_root_ptr(root_ptr);
  *this = std::move(temp);
}

ast_container &ast_container::operator=(const ast_container &other) {
  if (this == &other) return *this;
  ast_container temp = {other};
  *this = std::move(temp);
  return *this;
}

assignment_statement *ast_container::make_assignment_statement(variable_expression *left, i_ast_node *right,
                                                               location l) {
  return emplace_back<assignment_statement>(left, right, l);
}

binary_expression *ast_container::make_binary_expression(binary_operation op_type, i_ast_node *left, i_ast_node *right,
                                                         location l) {
  return emplace_back<binary_expression>(op_type, left, right, l);
}

constant_expression *ast_container::make_constant_expression(int val, location l) {
  return emplace_back<constant_expression>(val, l);
}

if_statement *ast_container::make_if_statement(i_ast_node *cond, i_ast_node *true_block, location l) {
  return emplace_back<if_statement>(cond, true_block, l);
}

if_statement *ast_container::make_if_statement(i_ast_node *cond, i_ast_node *true_block, i_ast_node *else_block,
                                               location l) {
  return emplace_back<if_statement>(cond, true_block, else_block, l);
}

print_statement *ast_container::make_print_statement(i_ast_node *expr, location l) {
  return emplace_back<print_statement>(expr, l);
}

// clang-format off
read_expression *ast_container::make_read_expression(location l) {
  return emplace_back<read_expression>(l);
}

error_node *ast_container::make_error_node(std::string msg, location l) {
  return emplace_back<error_node>(msg, l);
}
// clang-format on

statement_block *ast_container::make_statement_block(std::vector<i_ast_node *> &&vec, location l) {
  return emplace_back<statement_block>(std::move(vec), l);
}

statement_block *ast_container::make_statement_block(const std::vector<i_ast_node *> &vec, location l) {
  return make_statement_block(std::vector<i_ast_node *>{vec}, l);
}

unary_expression *ast_container::make_unary_expression(unary_operation op, i_ast_node *expr, location l) {
  return emplace_back<unary_expression>(op, expr, l);
}

variable_expression *ast_container::make_variable_expression(std::string &name, location l) {
  return emplace_back<variable_expression>(name, l);
}

while_statement *ast_container::make_while_statement(i_ast_node *cond, i_ast_node *block, location l) {
  return emplace_back<while_statement>(cond, block, l);
}

} // namespace paracl::frontend::ast