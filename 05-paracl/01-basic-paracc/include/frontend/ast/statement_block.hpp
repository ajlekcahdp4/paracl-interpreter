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
#include <vector>

namespace paracl::frontend::ast {

class statement_block : public i_statement_node {
public:
  std::vector<i_statement_node_uptr> m_statements;

  statement_block() = default;
  statement_block(std::vector<i_statement_node_uptr> &&vec) : m_statements{std::move(vec)} {}

  void append_statement(i_statement_node_uptr &&statement) { m_statements.emplace_back(std::move(statement)); }

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

static inline i_statement_node_uptr make_statement_block(std::vector<i_statement_node_uptr> &&vec) {
  return std::make_unique<statement_block>(std::move(vec));
}

} // namespace paracl::frontend::ast