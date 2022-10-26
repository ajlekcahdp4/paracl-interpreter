#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "bytecode_vm.hpp"
#include "bytecode_vm/disassembly.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "popl.hpp"

int main(int argc, char *argv[]) {
  using namespace paracl::bytecode_vm;
  std::string input_file_name;

  popl::OptionParser op("Allowed options");
  auto help_option = op.add<popl::Switch>("h", "help", "Print this help message");
  auto input_file_option = op.add<popl::Value<std::string>>("i", "input", "Specify input file");
  op.parse(argc, argv);

  if (help_option->is_set()) {
    std::cout << op << "\n";
    return 0;
  }

  if (!input_file_option->is_set()) {
    std::cerr << "File doesn't exist\n";
    return 1;
  }

  input_file_name = input_file_option->value();
  std::ifstream input_file;

  std::ios_base::iostate exception_mask = input_file.exceptions() | std::ios::failbit;
  input_file.exceptions(exception_mask);

  try {
    input_file.open(input_file_name, std::ios::binary);
  } catch (std::exception &e) {
    std::cerr << "Error opening file: " << e.what() << "\n";
  }

  auto ch = decl_vm::read_chunk(input_file);
  if (!ch) {
    std::cerr << "Encountered an unrecoverable error, existing...\n";
    return 1;
  }

  decl_vm::disassembly::chunk_complete_disassembler disas{instruction_set::paracl_isa};
  
  try {
    disas(std::cout, ch.value());
  } catch (std::exception &e) {
    std::cerr << "Encountered an unrecoverable error: " << e.what() << "\nExiting...\n";
    return 1;
  }

  return 0;
}