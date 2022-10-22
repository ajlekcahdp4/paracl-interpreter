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

#include <stdexcept>
#include <vector>

#include "bytecode_vm/chunk.hpp"
#include "bytecode_vm/opcodes.hpp"

#include "bytecode_vm/virtual_machine.hpp"

namespace paracl::bytecode_vm {

void virtual_machine::execute_instruction() {
  if (m_ip == m_ip_end) throw std::runtime_error{"Instruction pointer ran outside "};
  auto op = static_cast<opcode>(*m_ip);


}

} // namespace paracl::bytecode_vm