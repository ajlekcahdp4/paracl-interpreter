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

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "bytecode_vm.hpp"
#include "bytecode_vm/virtual_machine.hpp"
#include "frontend/ast.hpp"

namespace paracl::codegen {

class codegen_symtab_stack {
private:
  std::vector<std::unordered_map<std::string, uint32_t>> m_stack;

  uint32_t total_count = 0;

public:
  uint32_t lookup_location(const std::string &name) {
    for (int i = m_stack.size() - 1; i >= 0; --i) {
      const auto &elem = m_stack[i];
      auto        found = elem.find(name);
      if (found == elem.end()) continue;
      return found->second;
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

class codegen_visitor : public paracl::frontend::ast::i_ast_visitor {
  paracl::bytecode_vm::builder::bytecode_builder<decltype(paracl::bytecode_vm::instruction_set::paracl_isa)> m_builder;
  std::unordered_map<int, uint32_t> m_constant_map;
  codegen_symtab_stack              m_symtab_stack;

  bool m_prev_statement = false;

public:
  codegen_visitor() {}

  void visit(paracl::frontend::ast::assignment_statement *) override;
  void visit(paracl::frontend::ast::binary_expression *) override;
  void visit(paracl::frontend::ast::constant_expression *) override;
  void visit(paracl::frontend::ast::if_statement *) override;
  void visit(paracl::frontend::ast::print_statement *) override;
  void visit(paracl::frontend::ast::read_expression *) override;
  void visit(paracl::frontend::ast::statement_block *) override;
  void visit(paracl::frontend::ast::unary_expression *) override;
  void visit(paracl::frontend::ast::variable_expression *) override;
  void visit(paracl::frontend::ast::while_statement *) override;
  void visit(paracl::frontend::ast::error_node *) override;

  uint32_t lookup_or_insert_constant(int constant) {
    auto     found = m_constant_map.find(constant);
    uint32_t index;

    if (found == m_constant_map.end()) {
      index = m_constant_map.size();
      m_constant_map.insert({constant, index});
    }

    else {
      index = found->second;
    }

    return index;
  }

  paracl::bytecode_vm::decl_vm::chunk to_chunk() {
    using namespace paracl::bytecode_vm::builder;
    using namespace paracl::bytecode_vm::instruction_set;

    // Last instruction is ret
    m_builder.emit_operation(encoded_instruction{return_desc});

    auto ch = m_builder.to_chunk();

    std::vector<int> constants;
    constants.resize(m_constant_map.size());

    for (const auto &v : m_constant_map) {
      constants[v.second] = v.first;
    }

    ch.set_constant_pool(std::move(constants));
    return ch;
  }
};

paracl::bytecode_vm::decl_vm::chunk generate_code(paracl::frontend::ast::i_ast_node *);

} // namespace paracl::codegen