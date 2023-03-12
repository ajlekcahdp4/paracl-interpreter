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

#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/types/types.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace paracl::frontend {

class symtab final {
public:
  struct attributes {
    uint32_t m_loc;
    ast::variable_expression *m_definition;
  };

private:
  std::unordered_map<std::string, attributes> m_table;

public:
  void declare(std::string_view name, ast::variable_expression *def) {
    auto size = m_table.size();
    m_table.emplace(name, attributes{static_cast<uint32_t>(size), def});
  }

  bool declared(std::string_view name) const { return m_table.count(std::string{name}); }
  std::optional<attributes> get_attributes(std::string_view name) const {
    auto found = m_table.find(std::string{name});
    if (found == m_table.end()) return std::nullopt;
    return found->second;
  }

  // Deprecated, prefer attributes func
  std::optional<uint32_t> location(std::string_view name) const {
    auto found = m_table.find(std::string{name});
    if (found == m_table.end()) return std::nullopt;
    return found->second.m_loc;
  }

  auto begin() const { return m_table.begin(); }
  auto end() const { return m_table.end(); }
  auto size() const { return m_table.size(); }
};

class symtab_stack final : private std::vector<symtab *> {

public:
  void begin_scope(symtab *stab) { vector::push_back(stab); }
  void end_scope() { vector::pop_back(); }

  uint32_t size() const {
    return std::accumulate(vector::cbegin(), vector::cend(), 0, [](auto a, auto &&stab) { return a + stab->size(); });
  }

  uint32_t depth() const { return vector::size(); }

  uint32_t lookup_location(std::string_view name) const {
    uint32_t location = 0;
    auto found = std::find_if(vector::cbegin(), vector::cend(), [&name, &location](auto &stab) {
      auto loc = stab->location(name);
      if (loc.has_value()) {
        location += loc.value();
        return true;
      }
      location += stab->size();
      return false;
    });

    if (found == vector::cend()) {
      throw std::logic_error{"Trying to look up scope of a variable not present in symbol table"};
    }

    return location;
  }

  uint32_t lookup_scope(std::string_view name) const {
    auto found = std::find_if(vector::crbegin(), vector::crend(), [&name](auto &stab) { return stab->declared(name); });
    if (found == vector::crend()) {
      throw std::logic_error{"Trying to look up scope of a variable not present in symbol table"};
    }
    return std::distance(vector::crbegin(), found);
  }

  std::optional<symtab::attributes> lookup_symbol(std::string_view name) const {
    auto found = std::find_if(vector::crbegin(), vector::crend(), [&name](auto &stab) { return stab->declared(name); });
    if (found == vector::crend()) return std::nullopt;
    return (*found)->get_attributes(name);
  }

  bool declared(std::string_view name) const { return (lookup_symbol(name) ? true : false); }
  void declare(std::string_view name, ast::variable_expression *def) { vector::back()->declare(name, def); }
};

} // namespace paracl::frontend