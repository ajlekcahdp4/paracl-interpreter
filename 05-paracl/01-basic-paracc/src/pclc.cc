#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "bytecode_vm.hpp"
#include "codegen.hpp"
#include "frontend.hpp"
#include "frontend/dumper.hpp"
#include "frontend/semantic_analyzer.hpp"

#include "popl.hpp"

int main(int argc, char *argv[]) {
  using namespace paracl::bytecode_vm;
  using namespace instruction_set;
  using namespace decl_vm::disassembly;

  std::string input_file_name;
  bool        dump_binary = false;

  popl::OptionParser op("Allowed options");

  auto help_option = op.add<popl::Switch>("h", "help", "Print this help message");
  auto ast_dump_option = op.add<popl::Switch>("a", "ast-dump", "Dump AST");
  auto input_file_option = op.add<popl::Value<std::string>>("i", "input", "Specify input file");
  auto output_file_option = op.add<popl::Value<std::string>>("o", "output", "Specify output file for compiled program");
  auto disas_option = op.add<popl::Switch>("d", "disas", "Disassemble generated code (does not run the program)");

  op.parse(argc, argv);

  if (help_option->is_set()) {
    std::cout << op << "\n";
    return 0;
  }

  if (!input_file_option->is_set()) {
    std::cerr << "File not specified\n";
    return 1;
  }

  dump_binary = disas_option->is_set();

  input_file_name = input_file_option->value();
  std::ifstream input_file;

  std::ios_base::iostate exception_mask = input_file.exceptions() | std::ios::failbit;
  input_file.exceptions(exception_mask);

  try {
    input_file.open(input_file_name, std::ios::binary);
  } catch (std::exception &e) {
    std::cerr << "Error opening file: " << e.what() << "\n";
    return 1;
  }

  paracl::frontend::frontend_driver drv{};
  drv.switch_input_stream(&input_file);
  drv.parse();
  auto parse_tree = std::move(drv).take_ast();

  if (ast_dump_option->is_set()) {
    paracl::frontend::ast::ast_dump(parse_tree.get_root_ptr(), std::cout);
    return 0;
  }

  if (!ast_analyze(parse_tree.get_root_ptr())) {
    return 1;
  }

  auto ch = paracl::codegen::generate_code(parse_tree.get_root_ptr());

  if (dump_binary) {
    chunk_complete_disassembler disas{paracl_isa};
    disas(std::cout, ch);
    return 0;
  }

  if (output_file_option->is_set()) {
    std::string   output_file_name = output_file_option->value();
    std::ofstream output_file;

    output_file.exceptions(exception_mask);

    try {
      output_file.open(output_file_name, std::ios::binary);
    } catch (std::exception &e) {
      std::cerr << "Error opening output file: " << e.what() << "\n";
      return 1;
    }

    write_chunk(output_file, ch);
    return 0;
  }

  auto vm = create_paracl_vm();
  vm.set_program_code(std::move(ch));

  try {
    vm.execute(true);
  } catch (std::exception &e) {
    std::cerr << "Encountered an unrecoverable error: " << e.what() << "\nExiting...\n";
    return 1;
  }
}