/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "i_ast_node.hpp"

namespace paracl::frontend::ast {

enum class binary_operation {
  E_BIN_OP_ADD,
  E_BIN_OP_SUB,
  E_BIN_OP_MUL,
  E_BIN_OP_DIV,
  E_BIN_OP_MOD,
  E_BIN_OP_EQ,
  E_BIN_OP_NE,
  E_BIN_OP_GT,
  E_BIN_OP_LS,
  E_BIN_OP_GE,
  E_BIN_OP_LE,
  E_BIN_OP_AND,
  E_BIN_OP_OR,
};

static inline constexpr std::string_view binary_operation_to_string(binary_operation op) {
  using enum binary_operation;

  switch (op) {
  case E_BIN_OP_ADD: return "+";
  case E_BIN_OP_SUB: return "-";
  case E_BIN_OP_MUL: return "*";
  case E_BIN_OP_DIV: return "/";
  case E_BIN_OP_MOD: return "%";
  case E_BIN_OP_EQ: return "==";
  case E_BIN_OP_NE: return "!=";
  case E_BIN_OP_GT: return ">";
  case E_BIN_OP_LS: return "<";
  case E_BIN_OP_GE: return ">=";
  case E_BIN_OP_LE: return "<=";
  case E_BIN_OP_AND: return "&&";
  case E_BIN_OP_OR: return "||";
  }

  throw std::runtime_error{"Broken binary_operation enum"};
}

class binary_expression : public i_ast_node {
  binary_operation m_operation_type;
  i_ast_node_uptr  m_left, m_right;

public:
  binary_expression(binary_operation op_type, i_ast_node_uptr left, i_ast_node_uptr right, location l)
      : i_ast_node{l}, m_operation_type{op_type}, m_left{std::move(left)}, m_right{std::move(right)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  binary_operation op_type() const { return m_operation_type; }
  i_ast_node      *left() { return m_left.get(); }
  i_ast_node      *right() { return m_right.get(); }
};

static inline i_ast_node_uptr make_binary_expression(binary_operation op_type, i_ast_node_uptr left,
                                                     i_ast_node_uptr right, location l) {
  return std::make_unique<binary_expression>(op_type, std::move(left), std::move(right), l);
}

} // namespace paracl::frontend::ast