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

namespace paracl::frontend {

class semantic_analyzer_visitor : public ast::i_ast_visitor {
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
  void visit(ast::assignment_statement *) override;
  void visit(ast::binary_expression *) override;
  void visit(ast::constant_expression *) override;
  void visit(ast::if_statement *) override;
  void visit(ast::print_statement *) override;
  void visit(ast::read_expression *) override;
  void visit(ast::statement_block *) override;
  void visit(ast::unary_expression *) override;
  void visit(ast::variable_expression *) override;
  void visit(ast::while_statement *) override;
  void visit(ast::error_node *) override;

  bool valid() const { return m_valid; }
};

bool ast_analyze(ast::i_ast_node *node);

} // namespace paracl::frontend