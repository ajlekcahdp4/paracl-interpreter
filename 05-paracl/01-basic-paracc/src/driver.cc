#include <iostream>
#include <ostream>
#include <fstream>

#include "bytecode_vm.hpp"
#include "bytecode_vm/chunk.hpp"
#include "utils/serialization.hpp"
#include "bytecode_vm/disassembly.hpp"

int main() {
  using namespace paracl::bytecode_vm;

  constant_pool pool;
  pool.push_back(0);
  pool.push_back(42);
  pool.push_back(11);

  binary_code_buffer buf;

  std::ifstream is("./out.pcl", std::ios::binary);
  chunk ch = read_chunk(is).value();
  
  virtual_machine vm{ch};
  vm.execute();
  
  std::cout << "-------\n";
  disassembly::chunk_complete_disassembler{}(std::cout, ch);

  std::ofstream os("./copy.pcl", std::ios::binary);
  write_chunk(os, ch);
}