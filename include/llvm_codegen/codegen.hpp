/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/frontend_driver.hpp"

#include <llvm/IR/Module.h>

#include <memory>

namespace paracl::llvm_codegen {

namespace intrinsics {
void print(int32_t val);

int32_t read();
} // namespace intrinsics

auto emit_llvm(const frontend::frontend_driver &drv, llvm::LLVMContext &ctx) -> std::unique_ptr<llvm::Module>;

} // namespace paracl::llvm_codegen
