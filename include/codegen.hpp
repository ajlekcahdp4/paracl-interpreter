/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "ezvis/ezvis.hpp"

#include "bytecode_vm/bytecode_builder.hpp"
#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/disassembly.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "frontend/analysis/augmented_ast.hpp"
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"

#include "frontend/symtab.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace paracl::codegen {

class codegen_visitor final : public ezvis::visitor_base<frontend::ast::i_ast_node, codegen_visitor, void> {
  using builder_type = bytecode_vm::builder::bytecode_builder<decltype(bytecode_vm::instruction_set::paracl_isa)>;

private:
  std::unordered_map<int, uint32_t> m_constant_map;

  const frontend::ast::function_definition *m_curr_function;
  const frontend::types::builtin_types *m_types;

  struct reloc_constant {
    uint32_t m_index;
    uint32_t m_address;
  };

  std::vector<reloc_constant> m_return_address_constants;

  struct dyn_jump_reloc {
    uint32_t m_index;
    uint32_t m_address;
    frontend::ast::function_definition *m_func_ptr;
  };

  std::vector<dyn_jump_reloc> m_dynamic_jumps_constants;

  struct reloc_info {
    uint32_t m_reloc_index;
    frontend::ast::function_definition *m_func_ptr;
  };

  std::vector<reloc_info> m_relocations_function_calls;
  std::vector<uint32_t> m_exit_relocations;

  std::unordered_map<frontend::ast::function_definition *, uint32_t> m_function_defs;

  const frontend::functions_analytics *m_functions;
  frontend::symtab_stack m_symtab_stack;
  builder_type m_builder;

  bool m_is_currently_statement = false;

private:
  void set_currently_statement() { m_is_currently_statement = true; }
  void reset_currently_statement() { m_is_currently_statement = false; }
  bool is_currently_statement() const { return m_is_currently_statement; }

  void visit_if_no_else(frontend::ast::if_statement &);
  void visit_if_with_else(frontend::ast::if_statement &);

  uint32_t lookup_or_insert_constant(int constant);

  uint32_t current_constant_index() const {
    return m_constant_map.size() + m_return_address_constants.size() + m_dynamic_jumps_constants.size();
  }

  using to_visit = std::tuple<
      frontend::ast::assignment_statement, frontend::ast::binary_expression, frontend::ast::constant_expression,
      frontend::ast::if_statement, frontend::ast::print_statement, frontend::ast::read_expression,
      frontend::ast::statement_block, frontend::ast::unary_expression, frontend::ast::variable_expression,
      frontend::ast::while_statement, frontend::ast::function_call, frontend::ast::return_statement,
      frontend::ast::function_definition_to_ptr_conv, frontend::ast::i_ast_node>;

public:
  EZVIS_VISIT_CT(to_visit);

  codegen_visitor() = default;

  void generate(frontend::ast::assignment_statement &);
  void generate(frontend::ast::binary_expression &);
  void generate(frontend::ast::constant_expression &);
  void generate(frontend::ast::if_statement &);
  void generate(frontend::ast::print_statement &);
  void generate(frontend::ast::read_expression &);
  void generate(frontend::ast::statement_block &);
  void generate(frontend::ast::unary_expression &);
  void generate(frontend::ast::variable_expression &);
  void generate(frontend::ast::while_statement &);
  void generate(frontend::ast::function_call &);
  void generate(frontend::ast::return_statement &);
  void generate(frontend::ast::function_definition_to_ptr_conv &);

  void generate(const frontend::ast::i_ast_node &) {}
  uint32_t generate(frontend::ast::function_definition &);

  EZVIS_VISIT_INVOKER(generate);

  void generate_all(const frontend::ast::ast_container &ast, const frontend::functions_analytics &functions);

  bytecode_vm::decl_vm::chunk to_chunk();
};

bytecode_vm::decl_vm::chunk generate_code(frontend::ast::i_ast_node &);

} // namespace paracl::codegen