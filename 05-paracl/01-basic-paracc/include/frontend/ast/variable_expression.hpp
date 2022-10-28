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

namespace paracl::frontend::ast {

class variable_expression : public i_expression_node {
public:
  std::string m_name;

  variable_expression() = default;
  variable_expression(std::string p_name) : m_name{p_name} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(*this); }
};

static inline i_expression_node_uptr make_variable_expression(std::string name) {
  return std::make_unique<variable_expression>(name);
}

} // namespace paracl::frontend::ast