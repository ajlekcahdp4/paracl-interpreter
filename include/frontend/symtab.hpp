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

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace paracl::frontend {

class symtab final {
private:
  std::unordered_map<std::string, uint32_t> m_table;

public:
  void declare(std::string_view name) {
    auto size = m_table.size();
    m_table.emplace(name, size);
  }

  bool declared(std::string_view name) const { return m_table.count(std::string{name}); }

  std::optional<uint32_t> location(std::string_view name) const {
    auto found = m_table.find(std::string{name});
    if (found == m_table.end()) return std::nullopt;
    return found->second;
  }

  auto begin() const { return m_table.begin(); }

  auto end() const { return m_table.end(); }

  auto size() const { return m_table.size(); }
};

class symtab_stack final {
private:
  std::vector<symtab *> m_stack;

public:
  void begin_scope(symtab *stab) { m_stack.push_back(stab); }

  void end_scope() { m_stack.pop_back(); }

  uint32_t lookup_location(std::string_view name) const {
    uint32_t location = 0;
    auto found = std::find_if(m_stack.begin(), m_stack.end(), [&name, &location](auto &stab) {
      auto loc = stab->location(name);
      if (loc.has_value()) {
        location += loc.value();
        return true;
      }
      location += stab->size();
      return false;
    });

    if (found == m_stack.end())
      throw std::logic_error{"Trying to look up scope of a variable not present in symbol table"};
    return location;
  }

  uint32_t lookup_scope(std::string_view name) const {
    auto found = std::find_if(m_stack.rbegin(), m_stack.rend(), [&name](auto &stab) { return stab->declared(name); });
    if (found == m_stack.rend())
      throw std::logic_error{"Trying to look up scope of a variable not present in symbol table"};
    return std::distance(m_stack.rbegin(), found);
  }

  bool declared(std::string_view name) const {
    auto found = std::find_if(m_stack.begin(), m_stack.end(), [&name](auto &stab) { return stab->declared(name); });
    if (found == m_stack.end()) return false;
    return true;
  }

  void declare(std::string_view name) { m_stack.back()->declare(name); }
};

} // namespace paracl::frontend