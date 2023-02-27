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

#include "frontend/types/types.hpp"
#include "i_ast_node.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace paracl::frontend::ast {

class variable_expression final : public i_expression {
  std::string m_name;

public:
  EZVIS_VISITABLE();

  variable_expression(std::string p_name, types::shared_type type, location l) : i_expression{l}, m_name{p_name} {}
  variable_expression(std::string p_name, location l) : i_expression{l}, m_name{p_name} {}

  std::string type_str() const {
    if (!m_type) return "";
    return m_type->to_string();
  }

  std::string_view name() const & { return m_name; }
};

} // namespace paracl::frontend::ast