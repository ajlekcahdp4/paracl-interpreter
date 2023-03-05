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
#include "ezvis/ezvis.hpp"

namespace paracl::frontend {

class call_collector : public ezvis::visitor_base<ast::i_ast_node, call_collector, void> {
  std::vector<ast::i_ast_node *> *m_calls;

  using to_visit = std::tuple<
      ast::assignment_statement, ast::binary_expression, ast::if_statement, ast::print_statement, ast::statement_block,
      ast::unary_expression, ast::while_statement, ast::function_call, ast::i_ast_node>;

public:
  EZVIS_VISIT_CT(to_visit);

  void collect_calls(ast::assignment_statement &);
  void collect_calls(ast::binary_expression &);
  void collect_calls(ast::if_statement &);
  void collect_calls(ast::print_statement &);
  void collect_calls(ast::statement_block &);
  void collect_calls(ast::unary_expression &);
  void collect_calls(ast::while_statement &);
  void collect_calls(ast::return_statement &);
  void collect_calls(ast::function_call &);
  void collect_calls(ast::i_ast_node &) {}

  EZVIS_VISIT_INVOKER(collect_calls);

  void collect(ast::i_ast_node &node, std::vector<ast::i_ast_node *> &calls) {
    m_calls = &calls;
    apply(node);
  }
};

} // namespace paracl::frontend