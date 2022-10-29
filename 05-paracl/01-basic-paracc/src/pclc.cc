#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "frontend.hpp"

int main(int argc, char *argv[]) {
  paracl::frontend::frontend_driver drv{};
  drv.switch_input_stream(&std::cin);
  drv.parse();

  std::ofstream os("dump.txt");
  ast_dump(*drv.m_ast.get(), os);
}