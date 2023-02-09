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

#include "ast_copier.hpp"
#include "ast_nodes.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace paracl::frontend::ast {

using i_ast_node_uptr = std::unique_ptr<i_ast_node>;

class ast_container final {
private:
  std::vector<i_ast_node_uptr> m_nodes;
  i_ast_node                  *m_root = nullptr;

  friend class ast_copier;

  template <typename T, typename... Ts> T *emplace_back(Ts &&...args) {
    m_nodes.emplace_back(std::make_unique<T>(std::forward<Ts>(args)...));
    return static_cast<T *>(m_nodes.back().get());
  }

public:
  ast_container() = default;

  ast_container(const ast_container &other) {
    ast_container temp = {};
    auto          root_ptr = ast_copy(other.get_root_ptr(), temp);
    temp.set_root_ptr(root_ptr);
    *this = std::move(temp);
  }

  ast_container &operator=(const ast_container &other) {
    if (this == &other) return *this;
    ast_container temp = {other};
    *this = std::move(temp);
    return *this;
  }

  ast_container(ast_container &&other) = default;
  ast_container &operator=(ast_container &&other) = default;
  ~ast_container() = default;

  void        set_root_ptr(i_ast_node *ptr) { m_root = ptr; }
  i_ast_node *get_root_ptr() const { return m_root; }

  assignment_statement *make_assignment_statement(variable_expression *left, i_ast_node *right, location l) {
    return emplace_back<assignment_statement>(left, right, l);
  }

  binary_expression *make_binary_expression(binary_operation op_type, i_ast_node *left, i_ast_node *right, location l) {
    return emplace_back<binary_expression>(op_type, left, right, l);
  }

  constant_expression *make_constant_expression(int val, location l) {
    return emplace_back<constant_expression>(val, l);
  }

  if_statement *make_if_statement(i_ast_node *cond, i_ast_node *true_block, location l) {
    return emplace_back<if_statement>(cond, true_block, l);
  }

  if_statement *make_if_statement(i_ast_node *cond, i_ast_node *true_block, i_ast_node *else_block, location l) {
    return emplace_back<if_statement>(cond, true_block, else_block, l);
  }

  print_statement *make_print_statement(i_ast_node *expr, location l) { return emplace_back<print_statement>(expr, l); }

  // clang-format off
read_expression *make_read_expression(location l) {
  return emplace_back<read_expression>(l);
}

error_node *make_error_node(std::string msg, location l) {
  return emplace_back<error_node>(msg, l);
}
  // clang-format on

  statement_block *make_statement_block(std::vector<i_ast_node *> &&vec, location l) {
    return emplace_back<statement_block>(std::move(vec), l);
  }

  statement_block *make_statement_block(const std::vector<i_ast_node *> &vec, location l) {
    return make_statement_block(std::vector<i_ast_node *>{vec}, l);
  }

  unary_expression *make_unary_expression(unary_operation op, i_ast_node *expr, location l) {
    return emplace_back<unary_expression>(op, expr, l);
  }

  variable_expression *make_variable_expression(std::string &name, location l) {
    return emplace_back<variable_expression>(name, l);
  }

  while_statement *make_while_statement(i_ast_node *cond, i_ast_node *block, location l) {
    return emplace_back<while_statement>(cond, block, l);
  }
};

} // namespace paracl::frontend::ast

#include "ast_copier.hpp"