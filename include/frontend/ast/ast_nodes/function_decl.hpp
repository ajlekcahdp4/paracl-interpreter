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

class function_definition final : public i_ast_node {
private:
  // An optional function name. Those functions that don't have a name will be called anonymous functions
  std::optional<std::string> m_name;
  std::vector<variable_expression> m_arguments; // Argument list
  i_ast_node *m_block;

public:
  EZVIS_VISITABLE();

  function_definition(
      std::optional<std::string> name, i_ast_node &body, location l, std::vector<variable_expression> vars = {}
  )
      : i_ast_node{l}, m_name{name}, m_arguments{std::move(vars)}, m_block{&body} {}

  auto size() const { return m_arguments.size(); }

  auto begin() { return m_arguments.begin(); }
  auto end() { return m_arguments.end(); }
  auto begin() const { return m_arguments.cbegin(); }
  auto end() const { return m_arguments.cend(); }

  auto rbegin() const { return m_arguments.crbegin(); }
  auto rend() const { return m_arguments.crend(); }

  i_ast_node &body() const { return *m_block; }

  bool named() const { return m_name.has_value(); }

  std::string_view name() const {
    if (!named()) throw std::runtime_error{"Attempt to get name of the anonymous function"};
    return m_name.value();
  }
};

} // namespace paracl::frontend::ast