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

#include "ast/ast_nodes/i_ast_node.hpp"
#include "ezvis/ezvis.hpp"
#include "location.hpp"
#include "symtab.hpp"

#include <iostream>

namespace paracl::frontend {

class semantic_analyzer final : public ezvis::visitor_base<ast::i_ast_node, semantic_analyzer, void> {
private:
  symtab_stack m_scopes;
  bool m_valid = true;

  enum class semantic_analysis_state {
    E_LVALUE,
    E_RVALUE,
    E_DEFAULT,
  } current_state = semantic_analysis_state::E_DEFAULT;
  using to_visit = ast::tuple_ast_nodes;

  void set_state(semantic_analysis_state s) { current_state = s; }
  void reset_state() { current_state = semantic_analysis_state::E_DEFAULT; }

  void report_error(std::string msg, location loc) {
    m_valid = false;
    std::cerr << "Error at " << loc << " : " << msg << "\n";
  }

public:
  EZVIS_VISIT_CT(to_visit);

  // clang-format off
  void analyze_node(ast::read_expression &) { /* Do nothing */ }
  void analyze_node(ast::constant_expression &) { /* Do nothing */ }
  // clang-format on

  void analyze_node(ast::assignment_statement &);
  void analyze_node(ast::binary_expression &);

  void analyze_node(ast::if_statement &);
  void analyze_node(ast::print_statement &);

  void analyze_node(ast::statement_block &);
  void analyze_node(ast::unary_expression &);
  void analyze_node(ast::variable_expression &);
  void analyze_node(ast::while_statement &);
  void analyze_node(ast::error_node &);

  EZVIS_VISIT_INVOKER(analyze_node);

  bool analyze(ast::i_ast_node &root) {
    m_valid = true;
    apply(root);
    return m_valid;
  }
};

} // namespace paracl::frontend