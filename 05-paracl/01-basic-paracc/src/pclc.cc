#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "bytecode_vm.hpp"
#include "codegen.hpp"
#include "frontend.hpp"
#include "frontend/semantic_analyzer.hpp"

int main(int argc, char *argv[]) {
  paracl::frontend::frontend_driver drv{};
  drv.switch_input_stream(&std::cin);
  drv.parse();

  std::ofstream os("dump.txt");
  ast::ast_analyze(drv.m_ast.get());
  ast::ast_dump(drv.m_ast.get(), os);

  using namespace paracl::bytecode_vm;
  using namespace instruction_set;
  using namespace decl_vm::disassembly;

  auto ch = paracl::codegen::generate_code(drv.m_ast.get());

  chunk_complete_disassembler disas{paracl_isa};
  disas(std::cout, ch);

  auto vm = create_paracl_vm();
  vm.set_program_code(std::move(ch));

  try {
    vm.execute();
  } catch (std::exception &e) {
    std::cerr << "Encountered an unrecoverable error: " << e.what() << "\nExiting...\n";
    return 1;
  }
}