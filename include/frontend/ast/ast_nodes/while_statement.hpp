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

class while_statement final : public visitable_ast_node<while_statement> {
private:
  symtab      m_symtab;
  i_ast_node *m_condition;
  i_ast_node *m_block;

public:
  while_statement(i_ast_node *cond, i_ast_node *block, location l)
      : visitable_ast_node{l}, m_condition{cond}, m_block{block} {}

  while_statement(const while_statement &) = default;
  while_statement &operator=(const while_statement &) = default;

  i_ast_node *&cond() { return m_condition; }
  i_ast_node *&block() { return m_block; }

  symtab *symbol_table() { return &m_symtab; }
};

} // namespace paracl::frontend::ast