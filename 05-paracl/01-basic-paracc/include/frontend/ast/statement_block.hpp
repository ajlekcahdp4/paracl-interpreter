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

#include "i_statement_node.hpp"

namespace paracl::frontend::ast {

class statement_block : public i_statement_node {
  std::vector<i_statement_node_uptr> m_statements;

public:
  statement_block() = default;

  void append_statement(i_statement_node_uptr statement) { m_statements.emplace_back(statement.release()); }
};

using statement_block_uptr = std::unique_ptr<statement_block>;

} // namespace paracl::frontend::ast