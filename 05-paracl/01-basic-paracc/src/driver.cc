#include <iostream>

#include "bytecode_vm.hpp"

int main() {
  using namespace paracl::bytecode_vm;

  constant_pool pool;
  pool.push_back(0);
  pool.push_back(42);
  pool.push_back(11);

  binary_code_buffer buf;

  chunk ch{std::move(buf), std::move(pool)};
  ch.push_byte(opcode::E_PUSH_CONST_UNARY);
  ch.push_byte(2);
  ch.push_byte(0);
  ch.push_byte(0);
  ch.push_byte(0);

  std::cout << disassemble_chunk(ch) << "\n";
}