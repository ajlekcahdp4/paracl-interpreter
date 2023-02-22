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

#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace paracl::codegen {

class codegen_symtab_stack final {
private:
  std::vector<std::unordered_map<std::string, uint32_t>> m_stack;

  uint32_t total_count = 0;

public:
  uint32_t lookup_location(const std::string &name) const {
    for (auto its = m_stack.rbegin(), ite = m_stack.rend(); its != ite; ++its) {
      const auto &elem = *its;
      auto        found = elem.find(name);
      if (found != elem.end()) return found->second;
    }

    throw std::logic_error{"Codegen: trying to look up location of a variable not present in the symbol table"};
  }

  void begin_scope() { m_stack.emplace_back(); }

  void end_scope() {
    total_count -= m_stack.back().size();
    m_stack.pop_back();
  }

  void push_variable(const std::string &name) {
    m_stack.back().insert({name, total_count});
    total_count++;
  }
};

class codegen_visitor final : public ezvis::visitor_base<frontend::ast::i_ast_node, codegen_visitor, void> {
  using builder_type = bytecode_vm::builder::bytecode_builder<decltype(bytecode_vm::instruction_set::paracl_isa)>;

  std::unordered_map<int, uint32_t> m_constant_map;

  codegen_symtab_stack m_symtab_stack;
  builder_type         m_builder;

  bool m_is_currently_statement = false;

  void set_currently_statement();
  void reset_currently_statement();
  bool is_currently_statement() const;

  void visit_if_no_else(frontend::ast::if_statement &);
  void visit_if_with_else(frontend::ast::if_statement &);

  uint32_t lookup_or_insert_constant(int constant);

  using to_visit = frontend::ast::tuple_ast_nodes;

public:
  codegen_visitor() = default;

  EZVIS_VISIT_CT(to_visit);

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
  void generate(frontend::ast::error_node &);

  EZVIS_VISIT_INVOKER(generate);

  bytecode_vm::decl_vm::chunk to_chunk();
};

bytecode_vm::decl_vm::chunk generate_code(frontend::ast::i_ast_node &);

} // namespace paracl::codegen