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

using namespace paracl::utils::serialization;

namespace paracl::frontend::ast {

void ast_dump_visitor::visit(variable_expression &expr) {
  m_os << "\tnode_0x" << std::hex << pointer_to_uintptr(std::addressof(expr)) << " [label = \"<identifier> "
       << expr.m_name << "\" ];\n";
}

void ast_dump_visitor::visit(constant_expression &expr) {
  m_os << "\tnode_0x" << std::hex << pointer_to_uintptr(std::addressof(expr)) << " [label = \"<integer constant> "
       << std::dec << expr.m_val << "\" ];\n";
}

void ast_dump_visitor::visit(read_expression &expr) {
  m_os << "\tnode_0x" << std::hex << pointer_to_uintptr(std::addressof(expr)) << " [label = \"<read> ?\" ];\n";
}

void ast_dump_visitor::visit(binary_expression &expr) {
  m_os << std::hex;
  m_os << "\tnode_0x" << pointer_to_uintptr(std::addressof(expr)) << " [label = \"<binary_expression> "
       << ast::binary_operation_to_string(expr.m_operation_type) << "\" ];\n";

  m_os << "\tnode_0x" << pointer_to_uintptr(std::addressof(expr)) << " -> node_0x"
       << pointer_to_uintptr(expr.m_left.get()) << ";\n";

  m_os << "\tnode_0x" << pointer_to_uintptr(std::addressof(expr)) << " -> node_0x"
       << pointer_to_uintptr(expr.m_right.get()) << ";\n";

  ast_node_visit(*this, *expr.m_left.get());
  ast_node_visit(*this, *expr.m_right.get());
}

void ast_dump_visitor::visit(unary_expression &expr) {
  m_os << std::hex;

  m_os << "\tnode_0x" << pointer_to_uintptr(std::addressof(expr)) << " [label = \"<unary_expression> "
       << ast::unary_operation_to_string(expr.m_operation_type) << "\" ];\n";

  m_os << "\tnode_0x" << pointer_to_uintptr(std::addressof(expr)) << " -> node_0x"
       << pointer_to_uintptr(expr.m_expr.get()) << ";\n";

  ast_node_visit(*this, *expr.m_expr.get());
}

void ast_dump_visitor::visit(assignment_statement &) {}
void ast_dump_visitor::visit(if_statement &) {}
void ast_dump_visitor::visit(print_statement &) {}
void ast_dump_visitor::visit(statement_block &) {}
void ast_dump_visitor::visit(statement_expression &) {}
void ast_dump_visitor::visit(while_statement &) {}

void ast_dump(i_ast_node &node, std::ostream &os) {
  ast_dump_visitor dumper{os};

  os << "digraph AbstractSyntaxTree {\n";
  ast_node_visit(dumper, node);
  os << "}\n";
}

} // namespace paracl::frontend::ast