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
#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend::ast {

static void print_declare_node(std::ostream &os, i_ast_node *ptr, std::string_view label) {
  assert(ptr);
  os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(ptr) << " [label = \"" << label << "\" ];\n";
}

static void print_bind_node(std::ostream &os, i_ast_node *parent, i_ast_node *child, std::string_view label = "") {
  assert(parent);
  assert(child);
  os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(parent) << " -> node_0x"
     << utils::pointer_to_uintptr(child) << " [label = \"" << label << "\" ];\n";
}

void ast_dump_visitor::visit(variable_expression *ptr) {
  assert(ptr);
  std::stringstream ss;
  ss << "<identifier> " << ptr->name();
  print_declare_node(m_os, ptr, ss.str());
}

void ast_dump_visitor::visit(constant_expression *ptr) {
  assert(ptr);
  std::stringstream ss;
  ss << "<integer constant> " << std::dec << ptr->value();
  print_declare_node(m_os, ptr, ss.str());
}

// clang-format off

void ast_dump_visitor::visit(read_expression *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<read> ?");
}

// clang-format on

void ast_dump_visitor::visit(binary_expression *ptr) {
  assert(ptr);
  std::stringstream ss;
  ss << "<binary_expression> " << ast::binary_operation_to_string(ptr->op_type());
  print_declare_node(m_os, ptr, ss.str());
  print_bind_node(m_os, ptr, ptr->m_left);
  print_bind_node(m_os, ptr, ptr->m_right);

  ast_node_visit(*this, ptr->m_left);
  ast_node_visit(*this, ptr->m_right);
}

void ast_dump_visitor::visit(unary_expression *ptr) {
  assert(ptr);
  std::stringstream ss;
  ss << "<binary_expression> " << ast::unary_operation_to_string(ptr->op_type());
  print_declare_node(m_os, ptr, ss.str());
  print_bind_node(m_os, ptr, ptr->m_expr);

  ast_node_visit(*this, ptr->m_expr);
}

void ast_dump_visitor::visit(assignment_statement *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<assignment>");
  print_bind_node(m_os, ptr, ptr->m_left);
  print_bind_node(m_os, ptr, ptr->m_right);

  ast_node_visit(*this, ptr->m_left);
  ast_node_visit(*this, ptr->m_right);
}

void ast_dump_visitor::visit(if_statement *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<if>");
  print_bind_node(m_os, ptr, ptr->m_condition, "<condition>");
  print_bind_node(m_os, ptr, ptr->m_true_block, "<then>");

  ast_node_visit(*this, ptr->m_condition);
  ast_node_visit(*this, ptr->m_true_block);

  if (ptr->m_else_block) {
    print_bind_node(m_os, ptr, ptr->m_else_block, "<else>");
    ast_node_visit(*this, ptr->m_else_block);
  }
}

void ast_dump_visitor::visit(print_statement *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<print_statement>");
  print_bind_node(m_os, ptr, ptr->m_expr);
  ast_node_visit(*this, ptr->m_expr);
}

void ast_dump_visitor::visit(statement_block *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<statement_block>");

  for (const auto &v : *ptr) {
    print_bind_node(m_os, ptr, v);
    ast_node_visit(*this, v);
  }
}

void ast_dump_visitor::visit(while_statement *ptr) {
  assert(ptr);
  print_declare_node(m_os, ptr, "<while>");
  print_bind_node(m_os, ptr, ptr->m_condition, "<condition>");
  print_bind_node(m_os, ptr, ptr->m_block, "<body>");

  ast_node_visit(*this, ptr->m_condition);
  ast_node_visit(*this, ptr->m_block);
}

// clang-format off

void ast_dump_visitor::visit(error_node *ptr) {assert(ptr);
  print_declare_node(m_os, ptr, "<error>");
}

// clang-format on

void ast_dump(i_ast_node *node, std::ostream &os) {
  assert(node);
  ast_dump_visitor dumper{os};
  os << "digraph AbstractSyntaxTree {\n";
  ast_node_visit(dumper, node);
  os << "}\n";
}

} // namespace paracl::frontend::ast