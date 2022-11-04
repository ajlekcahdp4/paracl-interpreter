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

#include "i_ast_node.hpp"
#include "statement_block.hpp"
#include "symtab.hpp"

namespace paracl::frontend::ast {

class if_statement : public i_statement_node {
  symtab                 m_symtab;
  i_expression_node_uptr m_condition;
  i_statement_node_uptr  m_true_block;
  i_statement_node_uptr  m_else_block; // Optional, can be nullptr

public:
  if_statement(i_expression_node_uptr &&cond, i_statement_node_uptr &&true_block)
      : m_condition{std::move(cond)}, m_true_block{std::move(true_block)}, m_else_block{nullptr} {}

  if_statement(i_expression_node_uptr &&cond, i_statement_node_uptr &&true_block, i_statement_node_uptr &&else_block)
      : m_condition{std::move(cond)}, m_true_block{std::move(true_block)}, m_else_block{std::move(else_block)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  i_expression_node *cond() { return m_condition.get(); }
  i_statement_node  *true_block() { return m_true_block.get(); }
  i_statement_node  *else_block() { return m_else_block.get(); }
};

static inline i_statement_node_uptr make_if_statement(i_expression_node_uptr &&cond,
                                                      i_statement_node_uptr  &&true_block) {
  return std::make_unique<if_statement>(std::move(cond), std::move(true_block));
}

static inline i_statement_node_uptr make_if_statement(i_expression_node_uptr &&cond, i_statement_node_uptr &&true_block,
                                                      i_statement_node_uptr &&else_block) {
  return std::make_unique<if_statement>(std::move(cond), std::move(true_block), std::move(else_block));
}

} // namespace paracl::frontend::ast