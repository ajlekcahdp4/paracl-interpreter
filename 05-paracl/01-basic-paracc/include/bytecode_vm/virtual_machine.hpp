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

namespace paracl::bytecode_vm {

class virtual_machine {
private:
  const chunk      m_program_code;
  std::vector<int> m_execution_stack;

  binary_code_buffer::const_iterator m_ip;
  binary_code_buffer::const_iterator m_ip_end;

  bool halted = false;

  enum class compare_result {
    E_CMP_EQ,
    E_CMP_GT,
    E_CMP_LS,
  } compare_state = compare_result::E_CMP_EQ;

  bool execute_instruction();
  void report_error(std::string);

  std::optional<int> pop();
  void               push(int);

public:
  virtual_machine(const chunk &ch) : m_program_code{ch} {
    m_ip = m_program_code.m_binary_code.begin();
    m_ip_end = m_program_code.m_binary_code.end();
  }

  virtual_machine(chunk &&ch) : m_program_code{std::move(ch)} {
    m_ip = m_program_code.m_binary_code.begin();
    m_ip_end = m_program_code.m_binary_code.end();
  }

  bool execute() {
    while (!halted) {
      if (!execute_instruction()) return false;
    }
    return true;
  }
};

} // namespace paracl::bytecode_vm