/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ast/ast_nodes/i_ast_node.hpp"
#include "ezvis/ezvis.hpp"
#include "utils/serialization.hpp"

#include <cassert>
#include <iostream>

namespace paracl::frontend::ast {

class ast_dumper final : public ezvis::visitor_base<const i_ast_node, ast_dumper, void> {
private:
  using to_visit = tuple_ast_nodes;

private:
  std::ostream &m_os;

  static void print_declare_node(std::ostream &os, const i_ast_node &ref, std::string_view label) {
    os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(&ref) << " [label = \"" << label << "\" ];\n";
  }

  static void
  print_bind_node(std::ostream &os, const i_ast_node &parent, const i_ast_node &child, std::string_view label = "") {
    os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(&parent) << " -> node_0x"
       << utils::pointer_to_uintptr(&child) << " [label = \"" << label << "\" ];\n";
  }

public:
  explicit ast_dumper(std::ostream &os) : m_os{os} {}

  EZVIS_VISIT_CT(to_visit);

  void dump(const assignment_statement &);
  void dump(const binary_expression &);
  void dump(const constant_expression &);
  void dump(const if_statement &);
  void dump(const print_statement &);
  void dump(const read_expression &);
  void dump(const statement_block &);
  void dump(const unary_expression &);
  void dump(const variable_expression &);
  void dump(const while_statement &);
  void dump(const error_node &);
  void dump(const function_definition &);
  void dump(const return_statement &);
  void dump(const statement_block_expression &);

  EZVIS_VISIT_INVOKER(dump);
};

inline void ast_dump(i_ast_node &node, std::ostream &os) {
  ast_dumper dumper{os};
  os << "digraph abstract_syntax_tree {\n";
  dumper.apply(node);
  os << "}\n";
}

} // namespace paracl::frontend::ast