/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/dumper.hpp"
#include "frontend/frontend_driver.hpp"

#include "codegen.hpp"
#include "common.hpp"

#include <boost/program_options.hpp>

#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace po = boost::program_options;

int main(int argc, char *argv[]) try {
  auto desc = po::options_description{"Allowed options"};
  auto ast_dump_option = false;

  desc.add_options()("help", "produce help message");
  desc.add_options()("ast-dump,a", po::value(&ast_dump_option), "Dump AST");

  desc.add_options()("output,o", po::value(&n)->default_value(false), "Otput file for compiled program");

  po::positional_options_description pos_desc;
  pos_desc.add("input-file", -1);

  auto vm = po::variables_map{};
  po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);

  std::string input_file_name = vm["input-file"].as<std::string>();

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_FAILURE;
  }

  po::notify(vm);

  if (input_file_name.empty()) {
    fmt::println(stderr, "Input file must be specified");
    return EXIT_FAILURE;
  }

  paracl::frontend::frontend_driver drv{input_file_name};
  drv.parse();

  const auto &parse_tree = drv.ast();
  bool valid = drv.analyze();

  if (ast_dump_option->is_set()) {
    paracl::frontend::ast::ast_dump(parse_tree.get_root_ptr(), std::cout);
    return k_exit_success;
  }

  if (!valid) {
    return k_exit_failure;
  }

  paracl::codegen::codegen_visitor generator;
  generator.generate_all(drv.ast(), drv.functions());
  auto ch = generator.to_chunk();

  if (dump_binary) {
    disassemble_chunk(ch);
    return k_exit_success;
  }

  if (output_file_option->is_set()) {
    std::string output_file_name = output_file_option->value();
    std::ofstream output_file;
    utils::try_open_file(output_file, output_file_name, std::ios::binary);
    write_chunk(output_file, ch);
    return k_exit_success;
  }

  execute_chunk(ch);

} catch (std::exception &e) {
  fmt::println(stderr, "Error: {}", e.what());
  return k_exit_failure;
}
