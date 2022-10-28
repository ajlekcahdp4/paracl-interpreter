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

#include "i_expression_node.hpp"
#include <string>

namespace paracl::frontend::ast {

class variable_expression : public i_expression_node {
public:
  std::string m_name;

  variable_expression() = default;
  variable_expression(std::string p_name) : m_name{p_name} {}
};

static inline auto make_variable_expression(std::string name) {
  return i_expression_node_uptr{new variable_expression{name}};
}

} // namespace paracl::frontend::ast