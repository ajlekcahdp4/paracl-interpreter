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

#include "frontend/ast/ast_nodes.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace paracl::frontend {

class function_table final {
public:
  struct function_attributes {
    ast::function_definition *definition = nullptr;
    bool recursive = false;
  };

private:
  std::unordered_map<std::string, function_attributes> m_table;

public:
  std::optional<function_attributes> lookup(const std::string &name) {
    auto found = m_table.find(name);
    if (found == m_table.end()) return std::nullopt;
    return found->second;
  }

  std::pair<function_attributes, bool> define_function(const std::string &name, function_attributes attributes) {
    auto [iter, inserted] = m_table.emplace(std::make_pair(name, attributes));
    return std::make_pair(iter->second, inserted);
  }

  void set_recursive(const std::string &name) { m_table.at(name).recursive = true; }

  auto begin() { return m_table.begin(); }
  auto end() { return m_table.end(); }
  auto begin() const { return m_table.cbegin(); }
  auto end() const { return m_table.cend(); }
  auto size() const { return m_table.size(); }
};

} // namespace paracl::frontend