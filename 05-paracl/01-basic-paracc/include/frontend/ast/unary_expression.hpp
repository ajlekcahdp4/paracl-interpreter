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

enum class unary_operation {
  E_UN_OP_NEG,
  E_UN_OP_POS,
  E_UN_OP_NOT,
};

static inline constexpr std::string_view unary_operation_to_string(unary_operation op) {
  using enum unary_operation;

  switch (op) {
  case E_UN_OP_NEG: return "-";
  case E_UN_OP_POS: return "+";
  case E_UN_OP_NOT: return "!";
  }

  throw std::runtime_error{"Broken unary_operation enum"};
}

class unary_expression_node : public i_expression_node {
public:
  unary_operation        m_operation_type;
  i_expression_node_uptr m_expr;

  unary_expression_node(unary_operation op_type, i_expression_node_uptr p_expr)
      : m_operation_type{op_type}, m_expr{p_expr.release()} {}
};

} // namespace paracl::frontend::ast