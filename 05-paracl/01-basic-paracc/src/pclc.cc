#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "frontend.hpp"
#include "frontend/semantic_analyzer.hpp"

int main(int argc, char *argv[]) {
  paracl::frontend::frontend_driver drv{};
  drv.switch_input_stream(&std::cin);
  drv.parse();

  std::ofstream os("dump.txt");
  ast::ast_analyze(drv.m_ast.get());
  ast::ast_dump(drv.m_ast.get(), os);
}