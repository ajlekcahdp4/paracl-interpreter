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

#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/error.hpp"
#include "frontend/symtab.hpp"
#include "frontend/types/types.hpp"
#include "location.hpp"

#include <iostream>

namespace paracl::frontend {

class ftable_filler final : public ezvis::visitor_base<ast::i_ast_node, ftable_filler, void> {
private:
  std::vector<error_kind> *m_error_queue = nullptr;
  ast::ast_container *m_ast = nullptr;

  using to_visit = std::tuple<
      ast::assignment_statement, ast::binary_expression, ast::if_statement, ast::print_statement, ast::statement_block,
      ast::unary_expression, ast::while_statement, ast::function_definition, ast::function_definition_to_ptr_conv,
      ast::function_call, ast::i_ast_node>;

  void report_error(std::string msg, location loc) { m_error_queue->push_back(error_kind{msg, loc}); }

public:
  EZVIS_VISIT_CT(to_visit);

  void fill_ftable(ast::assignment_statement &);
  void fill_ftable(ast::binary_expression &);
  void fill_ftable(ast::if_statement &);
  void fill_ftable(ast::print_statement &);
  void fill_ftable(ast::statement_block &);
  void fill_ftable(ast::unary_expression &);
  void fill_ftable(ast::while_statement &);
  void fill_ftable(ast::function_definition &);
  void fill_ftable(ast::function_definition_to_ptr_conv &);
  void fill_ftable(ast::function_call &);
  void fill_ftable(ast::i_ast_node &) {}

  EZVIS_VISIT_INVOKER(fill_ftable);

  bool fill(ast::ast_container &ast, std::vector<error_kind> &errors) {
    errors.clear();
    m_error_queue = &errors;
    m_ast = &ast;

    apply(*m_ast->get_root_ptr());
    return errors.empty();
  }
};
} // namespace paracl::frontend