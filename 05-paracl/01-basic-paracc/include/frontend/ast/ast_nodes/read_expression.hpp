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

class read_expression : public i_ast_node {
public:
  read_expression(location l) : i_ast_node{l} {}

  read_expression(const read_expression &) = delete;
  read_expression &operator=(const read_expression &) = delete;

  void accept(i_ast_visitor &visitor) override { visitor.visit(this); }

  i_ast_node_uptr clone() override;
};

static inline i_ast_node_uptr make_read_expression(location l) { return std::make_unique<read_expression>(l); }

} // namespace paracl::frontend::ast