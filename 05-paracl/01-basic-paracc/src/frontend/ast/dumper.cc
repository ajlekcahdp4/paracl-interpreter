/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/ast/visitor/dumper.hpp"

#include "frontend/ast/unary_expression.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace paracl::utils::serialization;

namespace paracl::frontend::ast {

static void print_declare_node(std::ostream &os, i_ast_node *ptr, std::string_view label) {
  os << "\tnode_0x" << std::hex << pointer_to_uintptr(ptr) << " [label = \"" << label << "\" ];\n";
}

static void print_bind_node(std::ostream &os, i_ast_node *parent, i_ast_node *child) {
  os << "\tnode_0x" << std::hex << pointer_to_uintptr(parent) << " -> node_0x" << pointer_to_uintptr(child) << ";\n";
}

void ast_dump_visitor::visit(variable_expression *ptr) {
  std::stringstream ss;
  ss << "<identifier> " << ptr->name();
  print_declare_node(m_os, ptr, ss.str());
}

void ast_dump_visitor::visit(constant_expression *ptr) {
  std::stringstream ss;
  ss << "<integer constant> " << std::dec << ptr->value();
  print_declare_node(m_os, ptr, ss.str());
}

// clang-format off

void ast_dump_visitor::visit(read_expression *ptr) {
  print_declare_node(m_os, ptr, "<read> ?");
}

// clang-format on

void ast_dump_visitor::visit(binary_expression *ptr) {
  std::stringstream ss;
  ss << "<binary_expression> " << ast::binary_operation_to_string(ptr->op_type());
  print_declare_node(m_os, ptr, ss.str());
  print_bind_node(m_os, ptr, ptr->left());
  print_bind_node(m_os, ptr, ptr->right());

  ast_node_visit(*this, ptr->left());
  ast_node_visit(*this, ptr->right());
}

void ast_dump_visitor::visit(unary_expression *expr) {
  const auto base_ptr = static_cast<i_ast_node *>(expr);
  const auto child_base_ptr = static_cast<i_ast_node *>(expr->m_expr.get());

  m_os << std::hex;
  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " [label = \"<unary_expression> "
       << ast::unary_operation_to_string(expr->m_operation_type) << "\" ];\n";
  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " -> node_0x" << pointer_to_uintptr(child_base_ptr) << ";\n";

  ast_node_visit(*this, expr->m_expr.get());
}

void ast_dump_visitor::visit(assignment_statement *st) {
  const auto base_ptr = static_cast<i_ast_node *>(st);

  const auto left_child_base_ptr = static_cast<i_ast_node *>(st->m_left.get());
  const auto right_child_base_ptr = static_cast<i_ast_node *>(st->m_right.get());

  m_os << std::hex;

  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " [label = \"<assignment>\" ];\n";
  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " -> node_0x" << pointer_to_uintptr(left_child_base_ptr)
       << ";\n";

  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " -> node_0x" << pointer_to_uintptr(right_child_base_ptr)
       << ";\n";

  ast_node_visit(*this, st->m_left.get());
  ast_node_visit(*this, st->m_right.get());
}

void ast_dump_visitor::visit(if_statement *) {}

void ast_dump_visitor::visit(print_statement *st) {
  const auto base_ptr = static_cast<i_ast_node *>(st);
  const auto child_base_ptr = static_cast<i_ast_node *>(st->m_expr.get());

  m_os << std::hex;

  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " [label = \"<print_statement>\" ];\n";
  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " -> node_0x" << pointer_to_uintptr(child_base_ptr) << ";\n";

  ast_node_visit(*this, st->m_expr.get());
}

void ast_dump_visitor::visit(statement_block *st) {
  const auto base_ptr = static_cast<i_ast_node *>(st);

  m_os << std::hex;
  m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " [label = \"<statement_block>\" ];\n";

  for (const auto &v : st->m_statements) {
    m_os << std::hex;
    const auto child_base_ptr = static_cast<i_ast_node *>(v.get());
    m_os << "\tnode_0x" << pointer_to_uintptr(base_ptr) << " -> node_0x" << pointer_to_uintptr(child_base_ptr) << ";\n";
    ast_node_visit(*this, v.get());
  }
}

void ast_dump_visitor::visit(while_statement *) {}

void ast_dump(i_ast_node *node, std::ostream &os) {
  ast_dump_visitor dumper{os};

  os << "digraph AbstractSyntaxTree {\n";
  ast_node_visit(dumper, node);
  os << "}\n";
}

} // namespace paracl::frontend::ast