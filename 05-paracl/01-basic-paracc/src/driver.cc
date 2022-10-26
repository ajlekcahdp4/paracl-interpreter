#include <iostream>
#include <ostream>
#include <fstream>

#include "bytecode_vm.hpp"

int main() {
  using namespace paracl::bytecode_vm;

  decl_vm::constant_pool pool;
  pool.push_back(0);
  pool.push_back(42);
  pool.push_back(11);

  decl_vm::binary_code_buffer buf;

  decl_vm::chunk ch{decl_vm::binary_code_buffer{}, std::move(pool)};

  ch.push_byte(opcode::E_PUSH_CONST_UNARY);
  ch.push_value<uint32_t>(1);

  ch.push_byte(opcode::E_PUSH_CONST_UNARY);
  ch.push_value<uint32_t>(1);
  
  ch.push_byte(opcode::E_DIV_NULLARY);
  ch.push_byte(opcode::E_PRINT_NULLARY);
  ch.push_byte(opcode::E_RETURN_NULLARY);

  std::ofstream os("./copy.pcl", std::ios::binary);
  write_chunk(os, ch);

  decl_vm::virtual_machine vm{instruction_set::paracl_isa};
  
  vm.set_program_code(ch);
  vm.execute();
}