/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/semantic_analyzer.hpp"
#include "frontend/ast/ast_nodes.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend {

void semantic_analyzer::analyze_node(ast::unary_expression &ref) {
  apply(ref.expr());
}

void semantic_analyzer::analyze_node(ast::error_node &ref) {
  report_error(ref.error_msg(), ref.loc());
}

void semantic_analyzer::analyze_node(ast::assignment_statement &ref) {
  set_state(semantic_analysis_state::E_LVALUE);
  for (auto &v : ref) {
    apply(v);
  }

  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.right());
  reset_state();
}

void semantic_analyzer::analyze_node(ast::binary_expression &ref) {
  set_state(semantic_analysis_state::E_RVALUE);

  apply(ref.right());
  apply(ref.left());

  reset_state();
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.expr());
  reset_state();
}

void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  m_scopes.begin_scope(ref.symbol_table());

  for (auto &statement : ref) {
    assert(statement);
    apply(*statement);
  }

  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  m_scopes.begin_scope(ref.control_block_symtab());
  apply(ref.cond());

  m_scopes.begin_scope(ref.true_symtab());
  apply(ref.true_block());
  m_scopes.end_scope();

  if (ref.else_block() != nullptr) {
    m_scopes.begin_scope(ref.else_symtab());
    apply(*ref.else_block());
    m_scopes.end_scope();
  }

  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::while_statement &ref) {
  m_scopes.begin_scope(ref.symbol_table());

  apply(ref.cond());
  apply(ref.block());

  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::variable_expression &ref) {
  if (!m_scopes.declared(ref.name())) {
    if (current_state == semantic_analysis_state::E_LVALUE) {
      m_scopes.declare(ref.name());
    } else {
      report_error("Use of undeclared variable", ref.loc());
    }
  }
  /* TODO[Sergei]: Bind the variable to it's declaration symbol table */
}

void semantic_analyzer::analyze_node(ast::function_definition &ref) {}

void semantic_analyzer::analyze_node(ast::return_statement &ref) {}

void semantic_analyzer::analyze_node(ast::statement_block_expression &ref) {}

void semantic_analyzer::analyze_node(ast::function_call &ref) {}

} // namespace paracl::frontend