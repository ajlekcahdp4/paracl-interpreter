/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "common.hpp"

#include <boost/program_options.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char *argv[]) try {
  auto desc = po::options_description{"Allowed options"};
  std::string input_file_name;
  desc.add_options()("help", "produce help message");
  desc.add_options()("input-file", po::value(&input_file_name)->default_value("a.out"), "Input file name");

  po::positional_options_description pos_desc;
  pos_desc.add("input-file", -1);

  auto vm = po::variables_map{};
  po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  if (input_file_name.empty()) {
    fmt::println(stderr, "Input file must be specified");
    return EXIT_FAILURE;
  }

  std::ifstream input_file;
  utils::try_open_file(input_file, input_file_name, std::ios::binary);

  auto ch = paracl::bytecode_vm::decl_vm::read_chunk(input_file);
  if (!ch) {
    fmt::println(stderr, "Could not read input binary");
    return k_exit_failure;
  }
  execute_chunk(*ch);

  return k_exit_success;
} catch (std::exception &e) {
  fmt::println(stderr, "Error: {}", e.what());
  return k_exit_failure;
}
