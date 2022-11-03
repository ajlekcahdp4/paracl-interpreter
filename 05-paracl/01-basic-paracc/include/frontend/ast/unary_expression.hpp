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

class unary_expression : public i_expression_node {
  unary_operation        m_operation_type;
  i_expression_node_uptr m_expr;

public:
  unary_expression(unary_operation op_type, i_expression_node_uptr &&p_expr)
      : m_operation_type{op_type}, m_expr{std::move(p_expr)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  unary_operation    op_type() const { return m_operation_type; }
  i_expression_node *child() { return m_expr.get(); }
};

static inline i_expression_node_uptr make_unary_expression(unary_operation op, i_expression_node_uptr &&expr) {
  return std::make_unique<unary_expression>(op, std::move(expr));
}

} // namespace paracl::frontend::ast