#include <iostream>
#include <ostream>
#include <fstream>

#include "bytecode_vm.hpp"
#include "bytecode_vm/chunk.hpp"
#include "utils/serialization.hpp"
#include "bytecode_vm/disassembly.hpp"

#include "bytecode_vm/decl_vm.hpp"

int main() {
  using namespace paracl::bytecode_vm;

  constant_pool pool;
  pool.push_back(0);
  pool.push_back(42);
  pool.push_back(11);

  binary_code_buffer buf;

  chunk ch{binary_code_buffer{}, std::move(pool)};

  ch.push_opcode(opcode::E_PUSH_CONST_UNARY);
  ch.push_value<uint32_t>(1);

  ch.push_opcode(opcode::E_PUSH_CONST_UNARY);
  ch.push_value<uint32_t>(2);
  
  ch.push_opcode(opcode::E_DIV_NULLARY);
  ch.push_opcode(opcode::E_PRINT_NULLARY);
  ch.push_opcode(opcode::E_RETURN_NULLARY);

  // virtual_machine vm{ch};
  // vm.execute();
  
  // std::cout << "-------\n";
  // disassembly::chunk_complete_disassembler{}(std::cout, ch);

  // std::ofstream os("./copy.pcl", std::ios::binary);
  // write_chunk(os, ch);

  using namespace paracl::decl_vm;
  constexpr instruction_desc<static_cast<uint8_t>(opcode::E_PUSH_CONST_UNARY), unsigned> desc = "push_const";

  constexpr auto push_const = desc >> [](auto &&ctx, auto &&attr){ std::cout << std::get<0>(attr) << "\n"; };
  // std::cout << push_const.get_name();

  constexpr auto copy = push_const;

  auto first = ch.m_binary_code.begin() + 1;
  auto last = ch.m_binary_code.end();

  const auto set = paracl::decl_vm::instruction_set_description{push_const};

  paracl::decl_vm::virtual_machine vm{set};
  
  vm.set_program_code(ch);
  vm.execute_instruction();
}