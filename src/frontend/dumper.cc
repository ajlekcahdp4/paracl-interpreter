/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/dumper.hpp"
#include "frontend/ast/ast_nodes.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend::ast {

void ast_dumper::dump(error_node &ref) { print_declare_node(m_os, ref, "<error>"); }
void ast_dumper::dump(read_expression &ref) { print_declare_node(m_os, ref, "<read> ?"); }

void ast_dumper::dump(variable_expression &ref) {
  std::stringstream ss;
  ss << "<identifier> " << ref.name();
  print_declare_node(m_os, ref, ss.str());
}

void ast_dumper::dump(constant_expression &ref) {
  std::stringstream ss;
  ss << "<integer constant> " << std::dec << ref.value();
  print_declare_node(m_os, ref, ss.str());
}

void ast_dumper::dump(binary_expression &ref) {
  std::stringstream ss;
  ss << "<binary_expression> " << ast::binary_operation_to_string(ref.op_type());
  print_declare_node(m_os, ref, ss.str());

  print_bind_node(m_os, ref, ref.left());
  print_bind_node(m_os, ref, ref.right());

  apply(ref.left());
  apply(ref.right());
}

void ast_dumper::dump(unary_expression &ref) {
  std::stringstream ss;
  ss << "<binary_expression> " << ast::unary_operation_to_string(ref.op_type());

  print_declare_node(m_os, ref, ss.str());
  print_bind_node(m_os, ref, ref.expr());

  apply(ref.expr());
}

void ast_dumper::dump(assignment_statement &ref) {
  print_declare_node(m_os, ref, "<assignment>");
  const i_ast_node *prev = &ref;

  for (auto start = ref.begin(), finish = ref.end(); start != finish; ++start) {
    const auto curr_ptr = &(*start);

    apply(*curr_ptr);
    print_bind_node(m_os, *prev, *curr_ptr);

    prev = curr_ptr;
  }

  print_bind_node(m_os, ref, ref.right());
  apply(ref.right());
}

void ast_dumper::dump(if_statement &ref) {
  print_declare_node(m_os, ref, "<if>");

  print_bind_node(m_os, ref, ref.cond(), "<condition>");
  print_bind_node(m_os, ref, ref.true_block(), "<then>");

  apply(ref.cond());
  apply(ref.true_block());

  if (ref.else_block()) {
    print_bind_node(m_os, ref, *ref.else_block(), "<else>");
    apply(*ref.else_block());
  }
}

void ast_dumper::dump(print_statement &ref) {
  print_declare_node(m_os, ref, "<print_statement>");
  print_bind_node(m_os, ref, ref.expr());
  apply(ref.expr());
}

void ast_dumper::dump(statement_block &ref) {
  print_declare_node(m_os, ref, "<statement_block>");

  for (const auto &v : ref) {
    print_bind_node(m_os, ref, *v);
    apply(*v);
  }
}

void ast_dumper::dump(while_statement &ref) {
  print_declare_node(m_os, ref, "<while>");

  print_bind_node(m_os, ref, ref.cond(), "<condition>");
  print_bind_node(m_os, ref, ref.block(), "<body>");

  apply(ref.cond());
  apply(ref.block());
}

} // namespace paracl::frontend::ast