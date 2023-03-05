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
#include "ast/function_table.hpp"
#include "call_collector.hpp"
#include "dag/dag.hpp"

#include <iostream>
#include <string>

namespace paracl::frontend {

class callgraph : public graphs::dag<std::string> {
  ast::named_function_table *m_functions = nullptr;

public:
  callgraph() = delete;

  callgraph(ast::named_function_table *table) : m_functions{table} {
    auto &&ftable = *m_functions;
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
};

} // namespace paracl::frontend