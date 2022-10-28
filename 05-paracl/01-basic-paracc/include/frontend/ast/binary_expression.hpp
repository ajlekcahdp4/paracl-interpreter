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

#include "i_expression_node.hpp"

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
  E_BIN_OP_LE
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
  }

  throw std::runtime_error{"Broken binary_operation enum"};
}

class binary_expression_node : public i_expression_node {
public:
  binary_operation       m_operation_type;
  i_expression_node_uptr m_left, m_right;

  binary_expression_node(binary_operation op_type, i_expression_node_uptr left, i_expression_node_uptr right)
      : m_operation_type{op_type}, m_left{left.release()}, m_right{right.release()} {}
};

} // namespace paracl::frontend::ast