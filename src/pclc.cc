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

#include "llvm_codegen/codegen.hpp"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>

#include <boost/program_options.hpp>

#include <concepts>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {

enum class output_type : unsigned {
  BYTECODE,
  LLVM,
};

template <output_type T> static constexpr std::string_view output_type_name = "";

template <> constexpr std::string_view output_type_name<output_type::BYTECODE> = "bytecode";
template <> constexpr std::string_view output_type_name<output_type::LLVM> = "llvm";

auto derive_output_type(std::string_view type) {
  if (type == output_type_name<output_type::BYTECODE>) return output_type::BYTECODE;
  if (type == output_type_name<output_type::LLVM>) return output_type::LLVM;
  throw std::invalid_argument(fmt::format("Unknown output type: \"{}\"", type));
}

}; // namespace

namespace po = boost::program_options;

int main(int argc, char *argv[]) try {
  auto desc = po::options_description{"Allowed options"};
  auto ast_dump_option = false;
  std::string output_file_option;
  std::string input_file_name;
  std::string output_type_str;

  desc.add_options()("help", "Produce help message");
  desc.add_options()("emit-llvm", "Dump LLVM IR");
  desc.add_options()("ast-dump,a", po::value(&ast_dump_option)->default_value(false), "Dump AST");
  desc.add_options()("input-file", po::value(&input_file_name), "Input file name");
  desc.add_options()(
      "output-type,t",
      po::value(&output_type_str)->default_value(output_type_name<output_type::BYTECODE>.data()),
      "Output type"
  );

  desc.add_options()("output,o", po::value(&output_file_option), "Otput file for compiled program");
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

  paracl::frontend::frontend_driver drv{input_file_name};
  drv.parse();

  const auto &parse_tree = drv.ast();
  bool valid = drv.analyze();

  auto out_type = derive_output_type(output_type_str);

  if (ast_dump_option) {
    paracl::frontend::ast::ast_dump(parse_tree.get_root_ptr(), std::cout);
    return k_exit_success;
  }

  if (!valid) {
    return k_exit_failure;
  }

  if (out_type == output_type::LLVM) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::LLVMContext ctx;
    auto m = paracl::llvm_codegen::emit_llvm(drv, ctx);
    if (vm.count("emit-llvm")) m->dump();
    auto &module_ref = *m;
    std::string err;
    auto *exec = llvm::EngineBuilder(std::move(m))
                     .setEngineKind(llvm::EngineKind::JIT)
                     .setErrorStr(&err)
                     .create();
    if (!err.empty()) throw std::runtime_error(err);
    assert(exec);
    std::unordered_map<std::string, void *> external_functions;
    external_functions.try_emplace(
        "__print", reinterpret_cast<void *>(paracl::llvm_codegen::intrinsics::print)
    );
    external_functions.try_emplace(
        "__read", reinterpret_cast<void *>(paracl::llvm_codegen::intrinsics::read)
    );

    exec->InstallLazyFunctionCreator([&](const std::string &name) -> void * {
      auto it = external_functions.find(name);
      if (it == external_functions.end()) return nullptr;
      return it->second;
    });

    exec->finalizeObject();
    exec->setVerifyModules(true);
    exec->runFunction(module_ref.getFunction("main"), {});

    auto &Err = exec->getErrorMessage();
    if (!Err.empty()) throw std::runtime_error(Err);
    return EXIT_SUCCESS;
  }
  paracl::codegen::codegen_visitor generator;

  auto ch = generator.to_chunk();
  if (!output_file_option.empty()) {
    std::ofstream output_file;
    utils::try_open_file(output_file, output_file_option, std::ios::binary);
    write_chunk(output_file, ch);
    return EXIT_SUCCESS;
  }

  execute_chunk(ch);

} catch (std::exception &e) {
  fmt::println(stderr, "Error: {}", e.what());
  return k_exit_failure;
}
