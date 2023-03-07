/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/function_explorer.hpp"
#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"
#include "utils/misc.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend {

void function_explorer::explore(ast::assignment_statement &ref) {
  apply(ref.right());
}

void function_explorer::explore(ast::binary_expression &ref) {
  apply(ref.right());
  apply(ref.left());
}

void function_explorer::explore(ast::if_statement &ref) {
  apply(ref.cond());
  apply(ref.true_block());
  if (ref.else_block() != nullptr) apply(*ref.else_block());
}

void function_explorer::explore(ast::print_statement &ref) {
  apply(ref.expr());
}

void function_explorer::explore(ast::statement_block &ref) {
  for (auto &statement : ref) {
    assert(statement);
    apply(*statement);
  }
}

void function_explorer::explore(ast::unary_expression &ref) {
  apply(ref.expr());
}

void function_explorer::explore(ast::while_statement &ref) {
  apply(ref.cond());
  apply(ref.block());
}

void function_explorer::explore(ast::function_definition &ref) {
  auto &&name = ref.name();
  if (name.has_value()) {
    auto &&name_v = name.value();
    auto [ptr, inserted] = m_analytics->m_named.define_function(name_v, &ref);
    if (!inserted) {
      std::stringstream ss;
      ss << "Redefinition of function "
         << "\"" << name_v << "\""; // add information about previously declared function (in that case 'ptr' will
                                    // be a pointer to already declared function)
      report_error(ss.str(), ref.loc());
      return;
    }
    m_function_stack.push_back({name_v, &ref});
  } else {
    m_analytics->m_anonymous.define_function(&ref);
    std::stringstream ss;
    ss << "anonymous-" << m_analytics->m_anonymous.size();
    m_function_stack.push_back({ss.str(), &ref});
  }
  apply(ref.body());
  m_function_stack.pop_back();
}

void function_explorer::explore(ast::function_definition_to_ptr_conv &ref) {
  apply(ref.definition());
}

void function_explorer::explore(ast::function_call &ref) {
  auto &&curr_func = m_function_stack.back();
  auto *found = m_analytics->m_named.lookup(ref.name());
  m_analytics->m_callgraph.insert(curr_func, callgraph_value_type{std::string{ref.name()}, found});
  for (auto *param : ref)
    apply(*param);
}

void function_explorer::explore(ast::return_statement &ref) {
  apply(ref.expr());
}

} // namespace paracl::frontend