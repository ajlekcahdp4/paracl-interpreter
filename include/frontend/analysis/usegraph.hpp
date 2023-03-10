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

#include "frontend/ast/ast_nodes/function_decl.hpp"

#include "graphs/dag.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace paracl::frontend {

struct usegraph_value_type final {
  std::string m_name;
  ast::function_definition *m_definition;
};

inline bool operator==(const usegraph_value_type &lhs, const usegraph_value_type &rhs) {
  return lhs.m_name == rhs.m_name;
}

struct usegraph_hash {
  size_t operator()(paracl::frontend::usegraph_value_type x) const { return std::hash<std::string>{}(x.m_name); }
};

using usegraph = graphs::dag<usegraph_value_type, usegraph_hash>;

} // namespace paracl::frontend