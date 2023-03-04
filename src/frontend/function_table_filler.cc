/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/function_table_filler.hpp"
#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"
#include "utils/misc.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend {

void ftable_filler::fill_ftable(ast::assignment_statement &ref) {
  apply(ref.right());
}

void ftable_filler::fill_ftable(ast::binary_expression &ref) {
  apply(ref.right());
  apply(ref.left());
}

void ftable_filler::fill_ftable(ast::if_statement &ref) {
  apply(ref.cond());
  apply(ref.true_block());
  if (ref.else_block() != nullptr) apply(*ref.else_block());
}

void ftable_filler::fill_ftable(ast::print_statement &ref) {
  apply(ref.expr());
}

void ftable_filler::fill_ftable(ast::statement_block &ref) {
  for (auto &statement : ref) {
    assert(statement);
    apply(*statement);
  }
}

void ftable_filler::fill_ftable(ast::unary_expression &ref) {
  apply(ref.expr());
}

void ftable_filler::fill_ftable(ast::while_statement &ref) {
  apply(ref.cond());
  apply(ref.block());
}

void ftable_filler::fill_ftable(ast::function_definition &ref) {
  auto &&name = ref.name();
  if (name.has_value()) {
    auto [ptr, inserted] = m_ast->add_named_function(name.value(), &ref);
    if (!inserted) {
      std::stringstream ss;
      ss << "Redefinition of function "
         << "\"" << name.value() << "\""; // add information about previously declared function (in that case 'ptr' will
                                          // be a pointer to already declared function)
      report_error(ss.str(), ref.loc());
      return;
    }
  } else m_ast->add_anonymous_function(&ref);
}

void ftable_filler::fill_ftable(ast::function_definition_to_ptr_conv &ref) {
  apply(ref.definition());
}

void ftable_filler::fill_ftable(ast::function_call &ref) {
  for (auto *param : ref)
    apply(*param);
}

void ftable_filler::fill_ftable(ast::error_node &ref) {}

void ftable_filler::fill_ftable(ast::read_expression &ref) {}

void ftable_filler::fill_ftable(ast::constant_expression &ref) {}

void ftable_filler::fill_ftable(ast::variable_expression &ref) {}

} // namespace paracl::frontend