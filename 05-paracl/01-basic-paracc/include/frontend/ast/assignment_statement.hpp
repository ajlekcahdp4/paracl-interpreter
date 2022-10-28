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

class assignment_statement : public i_statement_node {
  i_expression_node_uptr m_left, m_right;

public:
  assignment_statement(i_expression_node_uptr left, i_expression_node_uptr right)
      : m_left{left.release()}, m_right{right.release()} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

} // namespace paracl::frontend::ast