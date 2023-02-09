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

class variable_expression final : public visitable_ast_node<variable_expression> {
  std::string m_name;

public:
  variable_expression(std::string p_name, location l) : visitable_ast_node{l}, m_name{p_name} {}
  std::string_view name() const & { return m_name; }
};

} // namespace paracl::frontend::ast