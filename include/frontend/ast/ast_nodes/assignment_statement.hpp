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
#include "variable_expression.hpp"
#include <cassert>
#include <vector>

namespace paracl::frontend::ast {

class assignment_statement final : public i_ast_node {
private:
  std::vector<variable_expression> m_lefts;
  i_ast_node *m_right;

public:
  EZVIS_VISITABLE();

  assignment_statement(variable_expression left, i_ast_node &right, location l) : i_ast_node{l}, m_right{&right} {
    m_lefts.push_back(left);
  }

  void append_variable(variable_expression var) {
    m_lefts.push_back(var);
    m_loc += var.loc();
  }

  auto size() const { return m_lefts.size(); }

  auto begin() { return m_lefts.rbegin(); }
  auto end() { return m_lefts.rend(); }
  auto begin() const { return m_lefts.crbegin(); }
  auto end() const { return m_lefts.crend(); }

  auto rbegin() const { return m_lefts.cbegin(); }
  auto rend() const { return m_lefts.cend(); }

  i_ast_node &right() const { return *m_right; }
};

} // namespace paracl::frontend::ast