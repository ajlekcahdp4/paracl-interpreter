/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "codegen.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace paracl::frontend::ast;
using namespace paracl::bytecode_vm::decl_vm;
using namespace paracl::bytecode_vm::builder;
using namespace paracl::bytecode_vm::instruction_set;

namespace paracl::codegen {

void codegen_visitor::visit(constant_expression *ptr) {
  uint32_t index = lookup_or_insert_constant(ptr->value());
  m_builder.emit_operation(encoded_instruction{push_const_desc, index});
}

// clang-format off

void codegen_visitor::visit(read_expression *) {
  m_builder.emit_operation(encoded_instruction{push_read_desc});
}

void codegen_visitor::visit(error_node *) {
  throw std::logic_error{"Calling codegeneration on an ill-formed program"};
}

void codegen_visitor::visit(variable_expression *ptr) {
  auto index = m_symtab_stack.lookup_location(std::string{ptr->name()});
  m_builder.emit_operation(encoded_instruction{push_local_desc, index});
}

// clang-format on

void codegen_visitor::visit(print_statement *ptr) {
  ast_node_visit(*this, ptr->expr());
  m_builder.emit_operation(encoded_instruction{print_desc});
}

void codegen_visitor::visit(assignment_statement *ptr) {
  bool emit_push = !m_prev_statement;
  m_prev_statement = false;

  ast_node_visit(*this, ptr->right());
  auto left_index = m_symtab_stack.lookup_location(std::string{ptr->left()->name()});

  m_builder.emit_operation(encoded_instruction{mov_local_desc, left_index});
  if (emit_push) m_builder.emit_operation(encoded_instruction{push_local_desc, left_index});
}

void codegen_visitor::visit(binary_expression *ptr) {
  ast_node_visit(*this, ptr->left());
  ast_node_visit(*this, ptr->right());

  using enum binary_operation;
  switch (ptr->op_type()) {
  case E_BIN_OP_ADD: m_builder.emit_operation(encoded_instruction{add_desc}); break;
  case E_BIN_OP_SUB: m_builder.emit_operation(encoded_instruction{sub_desc}); break;
  case E_BIN_OP_MUL: m_builder.emit_operation(encoded_instruction{mul_desc}); break;
  case E_BIN_OP_DIV: m_builder.emit_operation(encoded_instruction{div_desc}); break;
  case E_BIN_OP_MOD: m_builder.emit_operation(encoded_instruction{mod_desc}); break;
  case E_BIN_OP_EQ: break;
  case E_BIN_OP_NE: break;
  case E_BIN_OP_GT: break;
  case E_BIN_OP_LS: break;
  case E_BIN_OP_GE: break;
  case E_BIN_OP_LE: break;
  case E_BIN_OP_AND: break;
  case E_BIN_OP_OR: break;
  }
}

void codegen_visitor::visit(statement_block *ptr) {
  m_symtab_stack.begin_scope();

  for (const auto &v : *ptr->symbol_table()) {
    m_symtab_stack.push_variable(v);
    m_builder.emit_operation(encoded_instruction{push_const_desc, lookup_or_insert_constant(0)});
  }

  for (auto &statement : ptr->m_statements) {
    m_prev_statement = false;
    ast_node_visit(*this, statement.get());
  }

  for (uint32_t i = 0; i < ptr->symbol_table()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::visit(if_statement *ptr) {}

void codegen_visitor::visit(while_statement *ptr) {}

void codegen_visitor::visit(unary_expression *ptr) {
  using enum unary_operation;

  switch (ptr->op_type()) {
  case E_UN_OP_NEG: {
    m_builder.emit_operation(encoded_instruction{push_const_desc, lookup_or_insert_constant(0)});
    ast_node_visit(*this, ptr->child());
    m_builder.emit_operation(encoded_instruction{sub_desc});
  }

  case E_UN_OP_POS: {
    ast_node_visit(*this, ptr->child()); /* Do nothing */
    break;
  }

  case E_UN_OP_NOT: break;
  }
}

chunk generate_code(i_ast_node *ptr) {
  codegen_visitor generator{};
  ast_node_visit(generator, ptr);
  return generator.to_chunk();
}

} // namespace paracl::codegen