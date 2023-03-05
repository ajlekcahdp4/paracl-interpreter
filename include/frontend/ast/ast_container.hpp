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

#include "ast_copier.hpp"
#include "frontend/types/types.hpp"
#include "function_table.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace paracl::frontend::ast {

using i_ast_node_uptr = std::unique_ptr<i_ast_node>;

class ast_container final {
private:
  std::vector<i_ast_node_uptr> m_nodes;
  i_ast_node *m_root = nullptr;
  named_function_table m_function_table;
  anonymous_function_table m_anonymous_functions;
  types::builtin_types m_types;

  template <typename T, typename... Ts> T &emplace_back(Ts &&...args) {
    auto uptr = std::make_unique<T>(std::forward<Ts>(args)...);
    auto ptr = uptr.get();
    m_nodes.push_back(std::move(uptr));
    return *ptr;
  }

public:
  ast_container() = default;

  ast_container(const ast_container &other) {
    ast_container temp;
    auto root_ptr = ast_copy(other.get_root_ptr(), temp);
    temp.set_root_ptr(root_ptr);
    *this = std::move(temp);
  }

  ast_container &operator=(const ast_container &other) {
    if (this == &other) return *this;
    ast_container temp = {other};
    *this = std::move(temp);
    return *this;
  }

  ast_container(ast_container &&other) = default;
  ast_container &operator=(ast_container &&other) = default;
  ~ast_container() = default;

  void set_root_ptr(i_ast_node *ptr) { m_root = ptr; }

  i_ast_node *get_root_ptr() const & { return m_root; }

  template <typename t_node_type, typename... t_args>
  t_node_type &make_node(t_args &&...args)
    requires std::is_base_of_v<i_ast_node, t_node_type>
  {
    return emplace_back<t_node_type>(std::forward<t_args>(args)...);
  }

  auto void_type_ptr() { return m_types.m_void; }
  auto int_type_ptr() { return m_types.m_int; }
  types::builtin_types &builtin_types() & { return m_types; }

  i_ast_node *lookup_function(const std::string &name) { return m_function_table.lookup(name); }

  std::pair<i_ast_node *, bool> add_named_function(std::string_view name, i_ast_node *definition) {
    return m_function_table.define_function(name, definition);
  }

  void add_anonymous_function(i_ast_node *definition) { m_anonymous_functions.define_function(definition); }

  named_function_table &named_ftable() { return m_function_table; }
};

} // namespace paracl::frontend::ast