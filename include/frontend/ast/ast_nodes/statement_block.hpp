/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/symtab.hpp"
#include "i_ast_node.hpp"
#include "return_statement.hpp"

#include <cassert>
#include <vector>

namespace paracl::frontend::ast {

class statement_block : private std::vector<i_ast_node *>, public i_statement {
public:
  symtab stab;
  return_vector return_statements;

private:
  EZVIS_VISITABLE();

public:
  statement_block() = default;
  statement_block(std::vector<i_ast_node *> vec, location l) : vector{vec}, i_statement{l} {}

  void append_statement(i_ast_node &statement) {
    const bool empty = vector::empty();
    vector::push_back(&statement);

    if (empty) {
      m_loc = statement.loc();
    } else {
      m_loc += statement.loc();
    }
  }

  using vector::back;
  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::end;
  using vector::front;
  using vector::size;
};

class value_block : private std::vector<i_ast_node *>, public i_expression {
public:
  symtab stab;
  return_vector return_statements;

private:
  EZVIS_VISITABLE();

public:
  value_block() = default;
  value_block(std::vector<i_ast_node *> vec, location l) : vector{vec}, i_expression{l} {}

  void append_statement(i_ast_node &statement) {
    const bool empty = vector::empty();
    vector::push_back(&statement);

    if (empty) {
      m_loc = statement.loc();
    } else {
      m_loc += statement.loc();
    }
  }

  using vector::back;
  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::end;
  using vector::front;
  using vector::size;
};

} // namespace paracl::frontend::ast