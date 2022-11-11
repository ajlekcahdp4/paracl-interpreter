/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ast/visitor.hpp"
#include "symtab.hpp"

namespace paracl::frontend::ast {

class semantic_analyzer_visitor : public i_ast_visitor {
private:
  symtab_stack m_scopes;

  enum class semantic_analysis_state {
    E_LVALUE,
    E_RVALUE,
    E_DEFAULT,
  } current_state = semantic_analysis_state::E_DEFAULT;

  void set_state(semantic_analysis_state s) { current_state = s; }

  void report_error(std::string msg, location loc);
  void reset_state() { current_state = semantic_analysis_state::E_DEFAULT; }

  bool m_valid = true;

public:
  void visit(assignment_statement *) override;
  void visit(binary_expression *) override;
  void visit(constant_expression *) override;
  void visit(if_statement *) override;
  void visit(print_statement *) override;
  void visit(read_expression *) override;
  void visit(statement_block *) override;
  void visit(unary_expression *) override;
  void visit(variable_expression *) override;
  void visit(while_statement *) override;
  void visit(error_node *) override;

  bool valid() const { return m_valid; }
};

bool ast_analyze(i_ast_node *node);

} // namespace paracl::frontend::ast