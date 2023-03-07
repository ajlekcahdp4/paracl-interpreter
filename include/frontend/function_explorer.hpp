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

#include "callgraph.hpp"
#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/error.hpp"
#include "frontend/symtab.hpp"
#include "frontend/types/types.hpp"
#include "function_table.hpp"
#include "functions_analytics.hpp"
#include "location.hpp"

#include <iostream>
#include <string_view>

namespace paracl::frontend {

class function_explorer final : public ezvis::visitor_base<ast::i_ast_node, function_explorer, void> {
private:
  std::vector<callgraph_value_type> m_function_stack;
  std::vector<error_kind> *m_error_queue = nullptr;
  functions_analytics *m_analytics = nullptr;
  ast::ast_container *m_ast = nullptr;

  using to_visit = std::tuple<
      ast::assignment_statement, ast::binary_expression, ast::if_statement, ast::print_statement, ast::statement_block,
      ast::unary_expression, ast::while_statement, ast::function_definition, ast::function_definition_to_ptr_conv,
      ast::function_call, ast::return_statement, ast::i_ast_node>;

  void report_error(std::string msg, location loc) { m_error_queue->push_back(error_kind{msg, loc}); }

public:
  EZVIS_VISIT_CT(to_visit);

  void explore(ast::assignment_statement &);
  void explore(ast::binary_expression &);
  void explore(ast::if_statement &);
  void explore(ast::print_statement &);
  void explore(ast::statement_block &);
  void explore(ast::unary_expression &);
  void explore(ast::while_statement &);
  void explore(ast::function_definition &);
  void explore(ast::function_definition_to_ptr_conv &);
  void explore(ast::function_call &);
  void explore(ast::return_statement &);
  void explore(ast::i_ast_node &) {}

  EZVIS_VISIT_INVOKER(explore);

  bool explore(ast::ast_container &ast, std::vector<error_kind> &errors, functions_analytics &analytics) {
    errors.clear();
    m_function_stack.clear();
    m_ast = &ast;
    m_error_queue = &errors;
    m_analytics = &analytics;
    apply(*m_ast->get_root_ptr());
    return errors.empty();
  }
};
} // namespace paracl::frontend