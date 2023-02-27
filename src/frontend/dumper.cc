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

void ast_dumper::dump(const error_node &ref) {
  print_declare_node(m_os, ref, "<error>");
}

void ast_dumper::dump(const read_expression &ref) {
  print_declare_node(m_os, ref, "<read> ?");
}

void ast_dumper::dump(const variable_expression &ref) {
  std::stringstream ss;
  ss << "<identifier> " << ref.name() << "\n"
     << "<type> " << ref.type_str();
  print_declare_node(m_os, ref, ss.str());
}

void ast_dumper::dump(const constant_expression &ref) {
  std::stringstream ss;
  ss << "<integer constant> " << std::dec << ref.value();
  print_declare_node(m_os, ref, ss.str());
}

void ast_dumper::dump(const binary_expression &ref) {
  std::stringstream ss;
  ss << "<binary_expression> " << ast::binary_operation_to_string(ref.op_type());
  print_declare_node(m_os, ref, ss.str());

  print_bind_node(m_os, ref, ref.left());
  print_bind_node(m_os, ref, ref.right());

  apply(ref.left());
  apply(ref.right());
}

void ast_dumper::dump(const unary_expression &ref) {
  std::stringstream ss;
  ss << "<binary_expression> " << ast::unary_operation_to_string(ref.op_type());

  print_declare_node(m_os, ref, ss.str());
  print_bind_node(m_os, ref, ref.expr());

  apply(ref.expr());
}

void ast_dumper::dump(const assignment_statement &ref) {
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

void ast_dumper::dump(const if_statement &ref) {
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

void ast_dumper::dump(const print_statement &ref) {
  print_declare_node(m_os, ref, "<print_statement>");
  print_bind_node(m_os, ref, ref.expr());
  apply(ref.expr());
}

void ast_dumper::dump(const statement_block &ref) {
  print_declare_node(m_os, ref, "<statement_block>");

  for (const auto &v : ref) {
    print_bind_node(m_os, ref, *v);
    apply(*v);
  }
}

void ast_dumper::dump(const while_statement &ref) {
  print_declare_node(m_os, ref, "<while>");

  print_bind_node(m_os, ref, ref.cond(), "<condition>");
  print_bind_node(m_os, ref, ref.block(), "<body>");

  apply(ref.cond());
  apply(ref.block());
}

void ast_dumper::dump(const function_definition_to_ptr_conv &ref) {
  print_declare_node(m_os, ref, "<function def to ptr implicit conversion>");
  dump(ref.definition());
}

void ast_dumper::dump(const function_definition &ref) {
  std::stringstream ss;
  ss << "<function definition>: ";

  if (auto opt = ref.name(); opt) ss << opt.value() << "\n";
  else ss << "anonymous\n";
  ss << " <arg count>: " << ref.size() << "\n";
  ss << "<type> " << ref.type_str();

  print_declare_node(m_os, ref, ss.str());
  for (unsigned i = 0; const auto &v : ref) {
    ss.str("");
    ss << "arg " << i;
    print_bind_node(m_os, ref, v, ss.str());
    apply(v);
    ++i;
  }

  print_bind_node(m_os, ref, ref.body());
  apply(ref.body());
}

void ast_dumper::dump(const return_statement &ref) {
  print_declare_node(m_os, ref, "<return statement>");
  print_bind_node(m_os, ref, ref.expr(), "<expression>");
  apply(ref.expr());
}

void ast_dumper::dump(const function_call &ref) {
  std::stringstream ss;
  ss << "<function call>: " << ref.name();
  print_declare_node(m_os, ref, ss.str());
  ss << " <param count>: " << ref.size();

  for (unsigned i = 0; const auto &v : ref) {
    ss.str("");
    ss << "param " << i;
    print_bind_node(m_os, ref, *v, ss.str());
    apply(*v);
    ++i;
  }
}

} // namespace paracl::frontend::ast