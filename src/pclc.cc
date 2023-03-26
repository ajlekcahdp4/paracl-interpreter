#include "bytecode_vm/bytecode_builder.hpp"
#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/disassembly.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "codegen.hpp"
#include "frontend/ast/ast_copier.hpp"
#include "frontend/dumper.hpp"
#include "frontend/frontend_driver.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include "popl/popl.hpp"

#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {

constexpr int k_exit_success = 0;
constexpr int k_exit_failure = 1;
constexpr int k_exit_error = 2;

} // namespace

namespace utils = paracl::utils;

int main(int argc, char *argv[]) try {
  std::string input_file_name;
  bool dump_binary = false;

  popl::OptionParser op("Allowed options");

  auto help_option = op.add<popl::Switch>("h", "help", "Print this help message");
  auto ast_dump_option = op.add<popl::Switch>("a", "ast-dump", "Dump AST");
  auto input_file_option = op.add<popl::Implicit<std::string>>("i", "input", "Specify input file", "");
  auto output_file_option = op.add<popl::Value<std::string>>("o", "output", "Specify output file for compiled program");
  auto disas_option =
      op.add<popl::Switch>("d", "disas", "Disassemble generated code (does not run the program)", &dump_binary);

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

  paracl::frontend::frontend_driver drv{input_file_name};
  drv.parse();

  auto &parse_tree = drv.ast();

  if (!parse_tree.get_root_ptr()) {
    return k_exit_success;
  }

  auto valid = drv.analyze();

  if (ast_dump_option->is_set()) {
    paracl::frontend::ast::ast_dump(*parse_tree.get_root_ptr(), std::cout);
    return k_exit_success;
  }

  if (!valid) {
    return k_exit_failure;
  }

  using paracl::bytecode_vm::decl_vm::disassembly::chunk_complete_disassembler;
  namespace instruction_set = paracl::bytecode_vm::instruction_set;

  paracl::codegen::codegen_visitor generator;
  generator.generate_all(drv.ast(), drv.functions());
  auto ch = generator.to_chunk();

  if (dump_binary) {
    chunk_complete_disassembler disas{instruction_set::paracl_isa};
    disas(std::cout, ch);
    return k_exit_success;
  }

  if (output_file_option->is_set()) {
    std::string output_file_name = output_file_option->value();
    std::ofstream output_file;
    utils::try_open_file(output_file, output_file_name, std::ios::binary);
    write_chunk(output_file, ch);
    return k_exit_success;
  }

  auto vm = paracl::bytecode_vm::create_paracl_vm();
  vm.set_program_code(std::move(ch));
  vm.execute(true);

} catch (std::exception &e) {
  fmt::println(stderr, "Error: {}", e.what());
  return k_exit_error;
}