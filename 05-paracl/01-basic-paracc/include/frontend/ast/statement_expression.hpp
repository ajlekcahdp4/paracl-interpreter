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

class statement_expression : public i_statement_node {
  i_expression_node_uptr m_expr;

public:
  statement_expression(i_expression_node_uptr &&p_expr) : m_expr{std::move(p_expr)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

using statement_expression_uptr = std::unique_ptr<statement_expression>;

} // namespace paracl::frontend::ast