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
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::codegen {

namespace vm_instruction_set = bytecode_vm::instruction_set;
namespace vm_builder = bytecode_vm::builder;
namespace ast = frontend::ast;

using vm_builder::encoded_instruction;

void codegen_visitor::emit_pop() {
  emit_with_decrement(vm_instruction_set::pop_desc);
}

void codegen_visitor::generate(ast::constant_expression &ref) {
  auto index = lookup_or_insert_constant(ref.value());
  emit_with_increment(encoded_instruction{vm_instruction_set::push_const_desc, index});
}

void codegen_visitor::generate(ast::read_expression &ref) {
  emit_with_increment(vm_instruction_set::push_read_desc);
}

void codegen_visitor::generate(ast::variable_expression &ref) {
  auto index = m_symtab_stack.lookup_location(std::string{ref.name()});
  emit_with_increment(encoded_instruction{vm_instruction_set::push_local_rel_desc, index});
}

void codegen_visitor::generate(ast::print_statement &ref) {
  reset_currently_statement();
  apply(ref.expr());
  emit_with_decrement(vm_instruction_set::print_desc);
}

void codegen_visitor::generate(ast::assignment_statement &ref) {
  const bool emit_push = !is_currently_statement();
  apply(ref.right());

  const auto last_it = std::prev(ref.rend());
  for (auto start = ref.rbegin(), finish = last_it; start != finish; ++start) {
    const auto left_index = m_symtab_stack.lookup_location(std::string{start->name()});
    emit_with_decrement(encoded_instruction{vm_instruction_set::mov_local_rel_desc, left_index});
    emit_with_increment(encoded_instruction{vm_instruction_set::push_local_rel_desc, left_index});
  }

  // Last iteration:
  const auto left_index = m_symtab_stack.lookup_location(std::string{last_it->name()});
  emit_with_decrement(encoded_instruction{vm_instruction_set::mov_local_rel_desc, left_index});
  if (emit_push) {
    emit_with_increment(encoded_instruction{vm_instruction_set::push_local_rel_desc, left_index});
  }
}

void codegen_visitor::generate(ast::binary_expression &ref) {
  reset_currently_statement();
  apply(ref.left());

  reset_currently_statement();
  apply(ref.right());

  using bin_op = ast::binary_operation;

  switch (ref.op_type()) {
  case bin_op::E_BIN_OP_ADD: emit_with_decrement(vm_instruction_set::add_desc); break;
  case bin_op::E_BIN_OP_SUB: emit_with_decrement(vm_instruction_set::sub_desc); break;
  case bin_op::E_BIN_OP_MUL: emit_with_decrement(vm_instruction_set::mul_desc); break;
  case bin_op::E_BIN_OP_DIV: emit_with_decrement(vm_instruction_set::div_desc); break;
  case bin_op::E_BIN_OP_MOD: emit_with_decrement(vm_instruction_set::mod_desc); break;
  case bin_op::E_BIN_OP_EQ: emit_with_decrement(vm_instruction_set::cmp_eq_desc); break;
  case bin_op::E_BIN_OP_NE: emit_with_decrement(vm_instruction_set::cmp_ne_desc); break;
  case bin_op::E_BIN_OP_GT: emit_with_decrement(vm_instruction_set::cmp_gt_desc); break;
  case bin_op::E_BIN_OP_LS: emit_with_decrement(vm_instruction_set::cmp_ls_desc); break;
  case bin_op::E_BIN_OP_GE: emit_with_decrement(vm_instruction_set::cmp_ge_desc); break;
  case bin_op::E_BIN_OP_LE: emit_with_decrement(vm_instruction_set::cmp_le_desc); break;
  case bin_op::E_BIN_OP_AND: emit_with_decrement(vm_instruction_set::and_desc); break;
  case bin_op::E_BIN_OP_OR: emit_with_decrement(vm_instruction_set::or_desc); break;
  default: std::terminate();
  }
}

void codegen_visitor::generate(ast::statement_block &ref) {
  bool should_return = ref.type && ref.type != frontend::types::type_builtin::type_void();

  m_symtab_stack.begin_scope(&ref.stab);

  auto n_symbols = ref.stab.size();

  for (unsigned i = 0; i < n_symbols; ++i) {
    // no need to increment there cause it is already done in begin_scope
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

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
      bool is_statement_block = (node_type == frontend::ast::ast_node_type::E_STATEMENT_BLOCK);
      bool is_return = (node_type == frontend::ast::ast_node_type::E_RETURN_STATEMENT);
      bool is_last_iteration = start == last;
      bool pop_unused_result = (!is_last_iteration || !should_return) && is_raw_expression && !is_return;

      if (is_assignment && pop_unused_result) {
        set_currently_statement();
      } else {
        reset_currently_statement();
      }

      if (node_type != ast::ast_node_type::E_FUNCTION_DEFINITION) {
        apply(*statement);
      }

      if (!is_assignment && !is_statement_block && pop_unused_result) {
        if (!(node_type == frontend::ast::ast_node_type::E_FUNCTION_CALL &&
              static_cast<frontend::ast::function_call &>(*statement).type == frontend::types::type_builtin::type_void()
            )) {
          emit_pop();
        }
      }
      if (is_last_iteration && should_return && !is_return && is_raw_expression)
        emit_with_decrement(vm_instruction_set::load_r0_desc);
    }
  }

  for (unsigned i = 0; i < n_symbols; ++i) {
    emit_pop();
  }

  m_symtab_stack.end_scope();
  if (should_return) emit_with_increment(vm_instruction_set::store_r0_desc);
}

void codegen_visitor::visit_if_no_else(ast::if_statement &ref) {
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_false_block = emit_with_decrement(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

  set_currently_statement();
  apply(ref.true_block());

  auto jump_to_index = m_builder.current_loc();
  auto &to_relocate = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_false_block);

  std::get<0>(to_relocate.m_attr) = jump_to_index;
}

void codegen_visitor::visit_if_with_else(ast::if_statement &ref) {
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_false_block = emit_with_decrement(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

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
    // no need to increment there cause it is already done in begin_scope
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  if (!ref.else_block()) {
    visit_if_no_else(ref);
  } else {
    visit_if_with_else(ref);
  }

  for (unsigned i = 0; i < ref.control_block_symtab()->size(); ++i) {
    emit_pop();
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::generate(ast::while_statement &ref) {
  m_symtab_stack.begin_scope(ref.symbol_table());

  for (unsigned i = 0; i < ref.symbol_table()->size(); ++i) {
    // no need to increment there cause it is already done in begin_scope
    m_builder.emit_operation(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  auto while_location_start = m_builder.current_loc();
  reset_currently_statement();
  apply(ref.cond());

  auto index_jmp_to_after_loop = emit_with_decrement(encoded_instruction{vm_instruction_set::jmp_false_desc, 0});
  set_currently_statement();
  apply(ref.block());
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_desc, while_location_start});

  auto &to_relocate_after_loop_jump = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_after_loop);
  std::get<0>(to_relocate_after_loop_jump.m_attr) = m_builder.current_loc();

  for (unsigned i = 0; i < ref.symbol_table()->size(); ++i) {
    emit_pop();
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::generate(ast::unary_expression &ref) {
  using unary_op = ast::unary_operation;

  reset_currently_statement();
  switch (ref.op_type()) {
  case unary_op::E_UN_OP_NEG: {
    emit_with_increment(encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
    apply(ref.expr());
    emit_with_decrement(vm_instruction_set::sub_desc);
    break;
  }

  case unary_op::E_UN_OP_POS: {
    apply(ref.expr()); /* Do nothing */
    break;
  }

  case unary_op::E_UN_OP_NOT: {
    apply(ref.expr());
    m_builder.emit_operation(vm_instruction_set::not_desc);
    break;

  default: std::terminate();
  }
  }
}

void codegen_visitor::generate(ast::function_call &ref) {
  bool is_return;
  if (ref.type != frontend::types::type_builtin::type_void()) {
    is_return = true;
  }

  const auto const_index = current_constant_index();
  m_return_address_constants.push_back({const_index, 0}); // Dummy address
  const auto ret_addr_index = m_return_address_constants.size() - 1;

  m_symtab_stack.begin_scope(); // scope to isolate IP and SP
  emit_with_increment(encoded_instruction{vm_instruction_set::push_const_desc, const_index});

  emit_with_increment(encoded_instruction{vm_instruction_set::setup_call_desc});

  auto &&n_args = ref.size();
  for (const auto &e : ref) {
    assert(e);
    apply(*e);
  }
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::update_sp_desc, n_args});

  if (ref.m_def) {
    auto relocate_index = m_builder.emit_operation(encoded_instruction{vm_instruction_set::jmp_desc});
    m_relocations_function_calls.push_back({relocate_index, ref.m_def});
  } else {
    int total_depth = m_symtab_stack.size() - n_args;
    int index = m_symtab_stack.lookup_location(std::string{ref.name()});
    int rel_pos = index - total_depth;

    emit_with_increment(encoded_instruction{vm_instruction_set::push_local_rel_desc, rel_pos});
    emit_with_decrement(vm_instruction_set::jmp_dynamic_desc);
  }

  m_return_address_constants.at(ret_addr_index).m_address = m_builder.current_loc();
  m_symtab_stack.end_scope();
  if (is_return) emit_with_increment(encoded_instruction{vm_instruction_set::store_r0_desc});
}

void codegen_visitor::generate(frontend::ast::return_statement &ref) {
  if (!ref.empty()) {
    apply(ref.expr());
    emit_with_decrement(vm_instruction_set::load_r0_desc);
  }

  // clean up local variables
  auto local_var_n = m_symtab_stack.size();
  for (unsigned i = 0; i < local_var_n; ++i) {
    emit_pop();
  }

  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});
  decrement_stack();
  decrement_stack();
}

void codegen_visitor::generate(frontend::ast::function_definition_to_ptr_conv &ref) {
  const auto const_index = current_constant_index();
  m_dynamic_jumps_constants.push_back({const_index, 0, &ref.definition()}); // Dummy address
  emit_with_increment(encoded_instruction{vm_instruction_set::push_const_desc, const_index});
}

unsigned codegen_visitor::generate(frontend::ast::function_definition &ref) {
  m_symtab_stack.clear();

  m_curr_function = &ref;
  m_symtab_stack.begin_scope(&ref.param_symtab());

  const auto function_pos = m_builder.current_loc();
  m_function_defs.insert({&ref, function_pos});
  apply(ref.body());

  for (unsigned i = 0; i < ref.param_symtab().size(); ++i) {
    emit_pop();
  }

  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});
  decrement_stack();
  decrement_stack();

  m_symtab_stack.end_scope();

  return function_pos;
}

unsigned codegen_visitor::lookup_or_insert_constant(int constant) {
  auto found = m_constant_map.find(constant);
  unsigned index;

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
  m_functions = &functions;

  apply(*ast.get_root_ptr()); // Last instruction is ret
  m_builder.emit_operation(encoded_instruction{vm_instruction_set::return_desc});

  for (auto &func : functions.m_named) {
    generate(*func.second.definition);
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