/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ast_nodes.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace paracl::frontend::ast {

class named_function_table final {
  std::unordered_map<std::string, i_ast_node *> m_table;

public:
  i_ast_node *lookup(const std::string_view &name) {
    auto found = m_table.find(std::string{name});
    if (found == m_table.end()) return nullptr;
    return found->second;
  }

  std::pair<i_ast_node *, bool> define_function(std::string_view name, i_ast_node *definition) {
    auto [iter, inserted] = m_table.emplace(std::make_pair(std::string{name}, definition));
    return std::make_pair(iter->second, inserted);
  }

  auto begin() { return m_table.begin(); }
  auto end() { return m_table.end(); }
  auto begin() const { return m_table.cbegin(); }
  auto end() const { return m_table.cend(); }
  auto size() const { return m_table.size(); }
};

class anonymous_function_table final : private std::vector<i_ast_node *> {
public:
  void define_function(i_ast_node *definition) { vector::push_back(definition); }

  using vector::begin;
  using vector::end;
  using vector::operator[];
  using vector::size;
};

} // namespace paracl::frontend::ast