/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cstdint>
#include <string>

#include <vector>
#include <stdexcept>

#include "bytecode_vm/chunk.hpp"
#include "bytecode_vm/opcodes.hpp"

namespace paracl::bytecode_vm {

class virtual_machine {
private:
  const chunk                        m_program_code;
  std::vector<int>                   m_execution_stack;
  binary_code_buffer::const_iterator m_ip;
  binary_code_buffer::const_iterator m_ip_end;

  void execute_instruction();

public:
  virtual_machine(const chunk &ch) : m_program_code{} {}
  virtual_machine(chunk &&ch) : m_program_code{std::move(ch)} {}
};

} // namespace paracl::bytecode_vm