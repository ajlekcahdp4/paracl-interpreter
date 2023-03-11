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
#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::codegen {

namespace vm_instruction_set = bytecode_vm::instruction_set;
namespace vm_builder = bytecode_vm::builder;
namespace ast = frontend::ast;

using vm_builder::encoded_instruction;

void codegen_visitor::generate(ast::constant_expression &ref) {
  uint32_t index = lookup_or_insert_constant(ref.value());
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, index});
}

void codegen_visitor::generate(ast::read_expression &ref) {
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_read_desc});
}

void codegen_visitor::generate(ast::variable_expression &ref) {
  auto index = m_symtab_stack.lookup_location(std::string{ref.name()});
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_local_rel_desc, index});
}

void codegen_visitor::generate(ast::print_statement &ref) {
  reset_currently_statement();
  apply(ref.expr());
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::print_desc});
}

void codegen_visitor::generate(ast::assignment_statement &ref) {
  const bool emit_push = !is_currently_statement();
  apply(ref.right());

  const auto last_it = std::prev(ref.rend());
  for (auto start = ref.rbegin(), finish = last_it; start != finish; ++start) {
    const auto left_index = m_symtab_stack.lookup_location(std::string{start->name()});
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::mov_local_rel_desc, left_index});
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_local_rel_desc, left_index});
  }

  // Last iteration:
  const auto left_index = m_symtab_stack.lookup_location(std::string{last_it->name()});
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::mov_local_rel_desc, left_index});
  if (emit_push) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_local_rel_desc, left_index});
  }
}

void codegen_visitor::generate(ast::binary_expression &ref) {
  reset_currently_statement();
  apply(ref.left());

  reset_currently_statement();
  apply(ref.right());

  using bin_op = ast::binary_operation;

  switch (ref.op_type()) {
  case bin_op::E_BIN_OP_ADD: m_builder.emit_operation(encoded_instruction{vm_instruction_set::add_desc}); break;
  case bin_op::E_BIN_OP_SUB: m_builder.emit_operation(encoded_instruction{vm_instruction_set::sub_desc}); break;
  case bin_op::E_BIN_OP_MUL: m_builder.emit_operation(encoded_instruction{vm_instruction_set::mul_desc}); break;
  case bin_op::E_BIN_OP_DIV: m_builder.emit_operation(encoded_instruction{vm_instruction_set::div_desc}); break;
  case bin_op::E_BIN_OP_MOD: m_builder.emit_operation(encoded_instruction{vm_instruction_set::mod_desc}); break;
  case bin_op::E_BIN_OP_EQ: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_eq_desc}); break;
  case bin_op::E_BIN_OP_NE: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_ne_desc}); break;
  case bin_op::E_BIN_OP_GT: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_gt_desc}); break;
  case bin_op::E_BIN_OP_LS: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_ls_desc}); break;
  case bin_op::E_BIN_OP_GE: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_ge_desc}); break;
  case bin_op::E_BIN_OP_LE: m_builder.emit_operation(encoded_instruction{vm_instruction_set::cmp_le_desc}); break;
  case bin_op::E_BIN_OP_AND: m_builder.emit_operation(encoded_instruction{vm_instruction_set::and_desc}); break;
  case bin_op::E_BIN_OP_OR: m_builder.emit_operation(encoded_instruction{vm_instruction_set::or_desc}); break;
  }
}

void codegen_visitor::generate(ast::statement_block &ref) {
  m_scopes_depths.push_back(m_symtab_stack.depth());
  m_symtab_stack.begin_scope(ref.symbol_table());

  for (unsigned i = 0; i < ref.symbol_table()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  auto int_type = frontend::types::type_builtin{frontend::types::builtin_type_class::E_BUILTIN_INT};
  bool should_return = ref.m_type.get() && int_type.is_equal(*ref.m_type);

  if (ref.size()) {
    for (auto start = ref.cbegin(), finish = ref.cend(); start != finish; ++start) {
      auto &&last = std::prev(finish);
      auto &&statement = *start;

      assert(statement && "Broken statement pointer");
      using frontend::ast::ast_expression_types;

      const auto node_type = frontend::ast::identify_node(*statement);
      const auto is_raw_expression =
          std::find(ast_expression_types.begin(), ast_expression_types.end(), node_type) != ast_expression_types.end();
      bool is_assignment = (node_type == frontend::ast::ast_node_type::E_ASSIGNMENT_STATEMENT);
      bool pop_unused_result = ((start != last) || !should_return) && is_raw_expression;

      if (is_assignment && pop_unused_result) {
        set_currently_statement();
      } else {
        reset_currently_statement();
      }

      if (node_type != ast::ast_node_type::E_FUNCTION_DEFINITION) {
        apply(*statement);
      }

      if (!is_assignment && pop_unused_result) {
        if (!(node_type == frontend::ast::ast_node_type::E_FUNCTION_CALL &&
              static_cast<frontend::ast::function_call &>(*statement).m_type->is_equal(*m_types->m_void))) {
          m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
        }
      }
    }
  }

  for (uint32_t i = 0; i < ref.symbol_table()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::visit_if_no_else(ast::if_statement &ref) {
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_false_block = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

  set_currently_statement();
  apply(ref.true_block());

  auto jump_to_index = m_builder.current_loc();
  auto &to_relocate = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_false_block);

  std::get<0>(to_relocate.m_attr) = jump_to_index;
}

void codegen_visitor::visit_if_with_else(ast::if_statement &ref) {
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_false_block = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

  set_currently_statement();
  apply(ref.true_block());
  auto index_jmp_to_after_true_block = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_desc, 0});

  auto &to_relocate_else_jump = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_false_block);
  std::get<0>(to_relocate_else_jump.m_attr) = m_builder.current_loc();

  set_currently_statement();
  apply(*ref.else_block());

  auto &to_relocate_after_true_block = m_builder.get_as(vm_instruction_set::jmp_desc, index_jmp_to_after_true_block);
  std::get<0>(to_relocate_after_true_block.m_attr) = m_builder.current_loc();
}

void codegen_visitor::generate(ast::if_statement &ref) {
  m_symtab_stack.begin_scope(ref.control_block_symtab());

  for (unsigned i = 0; i < ref.control_block_symtab()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  m_scopes_depths.push_back(m_symtab_stack.depth());
  if (!ref.else_block()) {
    visit_if_no_else(ref);
  } else {
    visit_if_with_else(ref);
  }

  for (uint32_t i = 0; i < ref.control_block_symtab()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::generate(ast::while_statement &ref) {
  m_symtab_stack.begin_scope(ref.symbol_table());

  for (unsigned i = 0; i < ref.symbol_table()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  auto while_location_start = m_builder.current_loc();
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_after_loop = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});
  set_currently_statement();
  m_scopes_depths.push_back(m_symtab_stack.depth());
  apply(ref.block());
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_desc, while_location_start});

  auto &to_relocate_after_loop_jump = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_after_loop);
  std::get<0>(to_relocate_after_loop_jump.m_attr) = m_builder.current_loc();

  for (uint32_t i = 0; i < ref.symbol_table()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::generate(ast::unary_expression &ref) {
  using unary_op = ast::unary_operation;

  reset_currently_statement();
  switch (ref.op_type()) {
  case unary_op::E_UN_OP_NEG: {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
    apply(ref.expr());
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::sub_desc});
    break;
  }

  case unary_op::E_UN_OP_POS: {
    apply(ref.expr()); /* Do nothing */
    break;
  }

  case unary_op::E_UN_OP_NOT: {
    apply(ref.expr());
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::not_desc});
    break;
  }
  }
}

void codegen_visitor::generate(ast::function_call &ref) {
  bool is_return;
  if (!ref.m_type->is_equal(*m_types->m_void)) {
    is_return = true;
    uint32_t index = lookup_or_insert_constant(0);
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, index});
  }

  const auto const_index = current_constant_index();
  m_return_address_constants.push_back({const_index, 0}); // Dummy address
  const auto ret_addr_index = m_return_address_constants.size() - 1;
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, const_index});

  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_sp_desc});
  auto &&n_args = ref.size();
  for (const auto &e : ref) {
    assert(e);
    apply(*e);
  }
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::call_desc, n_args});

  if (ref.m_def) {
    uint32_t relocate_index = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_desc});
    m_relocations_function_calls.push_back({relocate_index, ref.m_def});
  } else {
    int32_t total_depth = m_symtab_stack.size() + 2 + (is_return ? 1 : 0);
    int32_t index = m_symtab_stack.lookup_location(std::string{ref.name()});
    int32_t rel_pos = index - total_depth - n_args;
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_local_rel_desc, rel_pos});
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_dynamic_desc});
  }

  m_return_address_constants[ret_addr_index].m_address = m_builder.current_loc();
}

void codegen_visitor::generate(frontend::ast::return_statement &ref) {
  if (!ref.empty()) {
    apply(ref.expr());
    // Move return value to the previous stack frame
    constexpr int32_t return_offset = -3;
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::mov_local_rel_desc, return_offset});
  }

  for (unsigned i = 0; i < m_symtab_stack.size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});

  auto &&prev_depth = m_scopes_depths.back();
  m_scopes_depths.pop_back();
  while (m_symtab_stack.depth() > prev_depth) {
    m_symtab_stack.end_scope();
  }
}

void codegen_visitor::generate(frontend::ast::function_definition_to_ptr_conv &ref) {
  const auto const_index = current_constant_index();
  m_dynamic_jumps_constants.push_back({const_index, 0, &ref.definition()}); // Dummy address
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, const_index});
}

uint32_t codegen_visitor::generate(frontend::ast::function_definition &ref) {
  m_curr_function = &ref;

  m_scopes_depths.push_back(m_symtab_stack.depth());
  m_symtab_stack.begin_scope(ref.param_symtab());

  const auto function_pos = m_builder.current_loc();
  m_function_defs.insert({&ref, function_pos});
  apply(ref.body());

  for (uint32_t i = 0; i < ref.param_symtab()->size(); ++i) {
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});
  m_symtab_stack.end_scope();
  return function_pos;
}

uint32_t codegen_visitor::lookup_or_insert_constant(int constant) {
  auto found = m_constant_map.find(constant);
  uint32_t index;

  if (found == m_constant_map.end()) {
    index = current_constant_index();
    m_constant_map.insert({constant, index});
  }

  else {
    index = found->second;
  }

  return index;
}

void codegen_visitor::generate_all(
    const frontend::ast::ast_container &ast, const frontend::functions_analytics &functions
) {
  m_return_address_constants.clear();

  m_types = &ast.builtin_types();
  m_functions = &functions;

  apply(*ast.get_root_ptr()); // Last instruction is ret
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});

  for (auto &func : functions.m_anonymous) {
    generate(*func);
  }

  for (auto &func : functions.m_named) {
    generate(*func.second);
  }

  for (const auto &reloc : m_relocations_function_calls) {
    auto &relocate_instruction = m_builder.get_as(vm_instruction_set::jmp_desc, reloc.m_reloc_index);
    relocate_instruction.m_attr = m_function_defs.at(reloc.m_func_ptr);
  }

  for (auto &dynjmp : m_dynamic_jumps_constants) {
    assert(dynjmp.m_func_ptr);
    dynjmp.m_address = m_function_defs.at(dynjmp.m_func_ptr);
  }
}

paracl::bytecode_vm::decl_vm::chunk codegen_visitor::to_chunk() {
  auto ch = m_builder.to_chunk();

  std::vector<int> constants;
  constants.resize(current_constant_index());

  for (const auto &v : m_constant_map) {
    constants[v.second] = v.first;
  }

  for (const auto &v : m_return_address_constants) {
    constants[v.m_index] = v.m_address;
  }

  for (const auto &v : m_dynamic_jumps_constants) {
    constants[v.m_index] = v.m_address;
  }

  ch.set_constant_pool(std::move(constants));
  return ch;
}

} // namespace paracl::codegen