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

#include "frontend/symtab.hpp"
#include "i_ast_node.hpp"
#include "statement_block.hpp"

namespace paracl::frontend::ast {

class if_statement : public i_ast_node {
  symtab          m_true_symtab;
  symtab          m_false_symtab;
  i_ast_node_uptr m_condition;
  i_ast_node_uptr m_true_block;
  i_ast_node_uptr m_else_block; // Optional, can be nullptr

public:
  if_statement(i_ast_node_uptr &&cond, i_ast_node_uptr &&true_block, location l)
      : i_ast_node{l}, m_condition{std::move(cond)}, m_true_block{std::move(true_block)}, m_else_block{nullptr} {}

  if_statement(i_ast_node_uptr &&cond, i_ast_node_uptr &&true_block, i_ast_node_uptr &&else_block, location l)
      : i_ast_node{l}, m_condition{std::move(cond)}, m_true_block{std::move(true_block)}, m_else_block{
                                                                                              std::move(else_block)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  i_ast_node *cond() { return m_condition.get(); }
  i_ast_node *true_block() { return m_true_block.get(); }
  i_ast_node *else_block() { return m_else_block.get(); }

  symtab *true_symtab() { return &m_true_symtab; }
  symtab *else_symtab() { return &m_false_symtab; }
};

static inline i_ast_node_uptr make_if_statement(i_ast_node_uptr &&cond, i_ast_node_uptr &&true_block, location l) {
  return std::make_unique<if_statement>(std::move(cond), std::move(true_block), l);
}

static inline i_ast_node_uptr make_if_statement(i_ast_node_uptr &&cond, i_ast_node_uptr &&true_block,
                                                i_ast_node_uptr &&else_block, location l) {
  return std::make_unique<if_statement>(std::move(cond), std::move(true_block), std::move(else_block), l);
}

} // namespace paracl::frontend::ast