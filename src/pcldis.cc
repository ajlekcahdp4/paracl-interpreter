#include "bytecode_vm/bytecode_builder.hpp"
#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/disassembly.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "utils/serialization.hpp"

#include "popl/popl.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

namespace {

constexpr int k_exit_success = 0;
constexpr int k_exit_failure = 1;
constexpr int k_exit_error = 2;

void disassemble_chunk(const paracl::bytecode_vm::decl_vm::chunk &ch) {
  using paracl::bytecode_vm::decl_vm::disassembly::chunk_complete_disassembler;
  namespace instruction_set = paracl::bytecode_vm::instruction_set;
  chunk_complete_disassembler disas{instruction_set::paracl_isa};
  disas(std::cout, ch);
}

} // namespace

namespace utils = paracl::utils;

int main(int argc, char *argv[]) try {
  std::string input_file_name;

  popl::OptionParser op("Allowed options");
  auto help_option = op.add<popl::Switch>("h", "help", "Print this help message");
  auto input_file_option = op.add<popl::Value<std::string>>("i", "input", "Specify input file");
  op.parse(argc, argv);

  if (help_option->is_set()) {
    fmt::println("{}", op.help());
    return k_exit_success;
  }

  if (!input_file_option->is_set()) {
    if (op.non_option_args().size() != 1) {
      fmt::println(stderr, "Input file not specified");
      return k_exit_failure;
    }

    input_file_name = op.non_option_args().front();
  } else {
    input_file_name = input_file_option->value();
  }

  std::ifstream input_file;
  utils::try_open_file(input_file, input_file_name, std::ios::binary);

  auto ch = paracl::bytecode_vm::decl_vm::read_chunk(input_file);
  if (!ch) {
    fmt::println(stderr, "Could not read input binary");
    return k_exit_failure;
  }

  disassemble_chunk(*ch);
  return k_exit_success;

} catch (std::exception &e) {
  fmt::println(stderr, "Error: {}", e.what());
  return k_exit_error;
}