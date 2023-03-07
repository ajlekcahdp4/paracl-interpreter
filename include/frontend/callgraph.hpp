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

#include "ast/ast_nodes.hpp"
#include <graphs/dag.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace paracl::frontend {

struct callgraph_value_type final {
  std::string m_name;
  ast::function_definition *m_definition;
};

inline bool operator==(const callgraph_value_type &lhs, const callgraph_value_type &rhs) {
  return lhs.m_name == rhs.m_name;
}
} // namespace paracl::frontend

namespace std {
template <> struct hash<paracl::frontend::callgraph_value_type> {
  size_t operator()(paracl::frontend::callgraph_value_type x) const { return std::hash<std::string>{}(x.m_name); }
};

} // namespace std

namespace paracl::frontend {

class callgraph : public graphs::dag<callgraph_value_type> {

public:
  callgraph() = default;
#if 0
  callgraph(ast::named_function_table *named_table) : m_named_functions{named_table} {

    auto &&ftable = *m_named_functions;
    call_collector collector;
    std::vector<ast::i_ast_node *> calls;

    for (auto &&definition : ftable) {
      calls.clear();
      auto *def_node = static_cast<ast::function_definition *>(definition.second);
      auto &&def_name = def_node->name();
      auto def_name_str = def_name.value();
      collector.collect(def_node->body(), calls);
      for (auto *call : calls) {
        auto *call_node = static_cast<ast::function_call *>(call);
        auto &&call_name = call_node->name();
        insert(def_name_str, std::string{call_name});
      }
    }
  }
#endif
};

} // namespace paracl::frontend