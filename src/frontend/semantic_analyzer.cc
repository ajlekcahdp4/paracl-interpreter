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
#include "frontend/ast/node_identifier.hpp"
#include "utils/misc.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend {

void semantic_analyzer::analyze_node(ast::unary_expression &ref) {
  apply(ref.expr());
  expect_type_eq(ref.expr(), *m_types->m_int);
  ref.set_type(m_types->m_int);
}

void semantic_analyzer::analyze_node(ast::error_node &ref) {
  report_error(ref.error_msg(), ref.loc());
}

void semantic_analyzer::analyze_node(ast::read_expression &ref) {
  ref.set_type(m_types->m_int);
}

void semantic_analyzer::analyze_node(ast::constant_expression &ref) {
  ref.set_type(m_types->m_int);
}

void semantic_analyzer::analyze_node(ast::assignment_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.right());

  set_state(semantic_analysis_state::E_LVALUE);
  auto &&right_type = ref.right().m_type;
  for (auto &v : ref) {
    auto &&declared = analyze_node(v);
    if (right_type.get() && !declared) {
      v.set_type(right_type);
    } else {
      expect_type_eq(v, *(ref.right().m_type));
    }
  }

  reset_state();
  ref.set_type(ref.right().get_type());
}

void semantic_analyzer::analyze_node(ast::binary_expression &ref) {
  set_state(semantic_analysis_state::E_RVALUE);

  apply(ref.right());
  apply(ref.left());
  expect_type_eq(ref.right(), *m_types->m_int);
  expect_type_eq(ref.left(), *m_types->m_int);
  ref.set_type(m_types->m_int);

  reset_state();
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);

  apply(ref.expr());
  expect_type_eq(ref.expr(), *m_types->m_int);

  reset_state();
}

void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  m_scopes.begin_scope(ref.symbol_table());

  for (auto &statement : ref) {
    assert(statement);
    apply(*statement);
  }

  if (current_state == semantic_analysis_state::E_RVALUE) {
    auto type = ezvis::visit_tuple<types::shared_type, ast::tuple_ast_nodes>(
        paracl::utils::visitors{
            [](ast::i_expression &expr) { return expr.get_type(); },
            [&](ast::i_ast_node &) { return m_types->m_void; }},
        *ref.back()
    );
    ref.set_type(type);
  }

  else {
    ref.set_type(m_types->m_void);
  }

  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  m_scopes.begin_scope(ref.control_block_symtab());
  apply(ref.cond());
  expect_type_eq(ref.cond(), *m_types->m_int);

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
  expect_type_eq(ref.cond(), *m_types->m_int);

  apply(ref.block());
  m_scopes.end_scope();
}

bool semantic_analyzer::analyze_node(ast::variable_expression &ref) {
  auto attr = m_scopes.lookup_symbol(ref.name());

  if (!attr) { // Not found.
    if (current_state == semantic_analysis_state::E_LVALUE) {
      m_scopes.declare(ref.name(), &ref);
      return false;
    }

    std::stringstream ss;
    ss << "Use of undeclared variable "
       << "'" << ref.name() << "'";
    report_error(ss.str(), ref.loc());

    return false;
  }

  assert(attr->definition && "Broken definition pointer");
  auto type = ezvis::visit<types::shared_type, ast::variable_expression>(
      [](ast::variable_expression &v) { return v.m_type; }, *attr->definition
  );
  ref.set_type(type);

  return true;
}

} // namespace paracl::frontend