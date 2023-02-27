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

public:
  EZVIS_VISITABLE();

  function_definition(
      std::optional<std::string> name, i_ast_node &body, location l, std::vector<variable_expression> vars = {}
  )
      : i_ast_node{l}, vector{std::move(vars)}, m_name{name}, m_block{&body} {}

  using vector::size;

  auto begin() const { return vector::begin(); }
  auto end() const { return vector::end(); }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;

  i_ast_node &body() const { return *m_block; }
  bool named() const { return m_name.has_value(); }

  std::optional<std::string> name() const {
    if (!m_name) return std::nullopt;
    return m_name;
  }
};

} // namespace paracl::frontend::ast