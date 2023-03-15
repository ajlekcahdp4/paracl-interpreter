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

#include "frontend/ast/ast_container.hpp"
#include "function_table.hpp"
#include "graphs/directed_graph.hpp"

#include <unordered_set>

namespace paracl::frontend {

using usegraph = graphs::basic_directed_graph<std::string, ast::function_definition *, void>;

struct functions_analytics final {
  named_function_table m_named;
  usegraph m_usegraph;
  std::unordered_set<ast::function_definition *> m_recursions;
};

} // namespace paracl::frontend