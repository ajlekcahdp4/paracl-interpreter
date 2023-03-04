/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/types/types.hpp"
#include "i_ast_node.hpp"
#include "variable_expression.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace paracl::frontend::ast {

class function_definition final : public i_ast_node, private std::vector<variable_expression> {
private:
  // An optional function name. Those functions that don't have a name will be called anonymous functions
  std::optional<std::string> m_name;
  i_ast_node *m_block;
  types::shared_type m_type;

public:
  EZVIS_VISITABLE();

  function_definition(
      std::optional<std::string> name, i_ast_node &body, location l, std::vector<variable_expression> vars = {}
  )
      : i_ast_node{l}, vector{std::move(vars)}, m_name{name}, m_block{&body} {}

  function_definition(
      std::optional<std::string> name, types::shared_type type, i_ast_node &body, location l,
      std::vector<variable_expression> vars = {}
  )
      : i_ast_node{l}, vector{std::move(vars)}, m_name{name}, m_block{&body}, m_type{type} {}

  using vector::size;

  auto begin() const { return vector::begin(); }
  auto end() const { return vector::end(); }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;

  i_ast_node &body() const { return *m_block; }
  bool named() const { return m_name.has_value(); }

  std::optional<std::string> name() const { return m_name; }

  std::string type_str() const {
    if (!m_type) return "";
    return m_type->to_string();
  }
};

class function_definition_to_ptr_conv final : public i_expression {
  function_definition *m_definition;

  EZVIS_VISITABLE();

public:
  function_definition_to_ptr_conv(location l, function_definition &def) : i_expression{l}, m_definition{&def} {}
  function_definition &definition() const { return *m_definition; }
};

} // namespace paracl::frontend::ast