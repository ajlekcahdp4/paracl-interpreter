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
#include "bytecode_vm/decl_vm.hpp"

namespace paracl::bytecode_vm {

namespace instruction_set {

using namespace paracl::decl_vm;

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_PUSH_CONST_UNARY), unsigned> push_const_desc = "push_const";
constexpr auto push_const_instr = push_const_desc >> [](auto &&ctx, auto &&attr){ ctx.push(ctx.pool().at(std::get<0>(attr))); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_RETURN_NULLARY), unsigned> return_desc = "ret";
constexpr auto return_instr = return_desc >> [](auto &&ctx, auto &&){ ctx.halt(); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_RETURN_NULLARY), unsigned> pop_desc = "pop";
constexpr auto pop_instr = return_desc >> [](auto &&ctx, auto &&){ ctx.pop(); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_ADD_NULLARY), unsigned> add_desc = "add";
constexpr auto add_instr = return_desc >> [](auto &&ctx, auto &&){ auto second = ctx.pop(); auto first = ctx.pop(); ctx.push(first + second); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_SUB_NULLARY), unsigned> sub_desc = "sub";
constexpr auto sub_instr = return_desc >> [](auto &&ctx, auto &&){ auto second = ctx.pop(); auto first = ctx.pop(); ctx.push(first - second); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_MUL_NULLARY), unsigned> mul_desc = "mul";
constexpr auto mul_instr = return_desc >> [](auto &&ctx, auto &&){ auto second = ctx.pop(); auto first = ctx.pop(); ctx.push(first * second); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_MUL_NULLARY), unsigned> div_desc = "mul";
constexpr auto div_instr = return_desc >> [](auto &&ctx, auto &&){ auto second = ctx.pop(); auto first = ctx.pop(); ctx.push(first / second); };

constexpr instruction_desc<static_cast<uint8_t>(opcode::E_MUL_NULLARY), unsigned> mod_desc = "mul";
constexpr auto mod_instr = return_desc >> [](auto &&ctx, auto &&){ auto second = ctx.pop(); auto first = ctx.pop(); ctx.push(first % second); };


}

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

  bool execute_nullary(const nullary_instruction &nullary);
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