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
#include <vector>

namespace paracl::frontend::ast {

class statement_block : public i_ast_node {
private:
  symtab m_symtab;

public:
  std::vector<i_ast_node *> m_statements;

  statement_block(std::vector<i_ast_node *> vec, location l) : i_ast_node{l}, m_statements{vec} {}

  statement_block(const statement_block &) = delete;
  statement_block &operator=(const statement_block &) = delete;

  void append_statement(i_ast_node *statement) { m_statements.push_back(statement); }

  void accept(i_ast_visitor &visitor) override { visitor.visit(this); }

  i_ast_node *clone() override;

  symtab *symbol_table() { return &m_symtab; }
};

} // namespace paracl::frontend::ast