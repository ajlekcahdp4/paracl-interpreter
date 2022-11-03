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

#include "statement_block.hpp"

namespace paracl::frontend::ast {

class while_statement : public i_statement_node, public i_expression_node {
  i_expression_node_uptr m_condition;
  i_statement_node_uptr  m_block;

public:
  while_statement(i_expression_node_uptr cond, i_statement_node_uptr block)
      : m_condition{cond.release()}, m_block{block.release()} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }
};

} // namespace paracl::frontend::ast