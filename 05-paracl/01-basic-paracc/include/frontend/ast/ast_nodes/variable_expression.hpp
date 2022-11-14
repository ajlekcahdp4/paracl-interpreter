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
#include <string>
#include <string_view>

namespace paracl::frontend::ast {

class variable_expression : public i_ast_node {
  std::string m_name;

public:
  variable_expression(std::string p_name, location l) : i_ast_node{l}, m_name{p_name} {}

  variable_expression(const variable_expression &) = default;
  variable_expression &operator=(const variable_expression &) = default;

  void accept(i_ast_visitor &visitor) override { visitor.visit(this); }

  std::string_view name() const & { return m_name; }
};

using variable_expression_uptr = std::unique_ptr<variable_expression>;

} // namespace paracl::frontend::ast