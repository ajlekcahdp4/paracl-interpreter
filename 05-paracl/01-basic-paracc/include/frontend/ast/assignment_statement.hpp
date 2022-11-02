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

class assignment_statement : public i_statement_node, public i_expression_node {
  std::string            m_left;
  i_expression_node_uptr m_right;

public:
  assignment_statement(std::string left, i_expression_node_uptr &&right) : m_left{left}, m_right{std::move(right)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

static inline i_statement_node_uptr make_assignment_statement(std::string left, i_expression_node_uptr &&right) {
  return std::make_unique<assignment_statement>(left, std::move(right));
}

} // namespace paracl::frontend::ast