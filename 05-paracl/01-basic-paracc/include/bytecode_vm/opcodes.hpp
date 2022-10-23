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

#include <unordered_map>

namespace paracl::bytecode_vm {

// clang-format off
enum class opcode : std::uint8_t {
  E_RETURN_NULLARY,

  E_PUSH_CONST_UNARY,
  E_POP_NULLARY,

  E_ADD_NULLARY, E_SUB_NULLARY, E_MUL_NULLARY,
  E_DIV_NULLARY, E_MOD_NULLARY, E_PRINT_NULLARY,
  E_PUSH_READ_NULLARY, E_PUSH_LOCAL_UNARY, E_MOV_LOCAL_UNARY,

  E_CMP_NULLARY, E_JMP_ABS_UNARY,
  E_JMP_EQ_ABS_UNARY, E_JMP_NE_ABS_UNARY, E_JMP_GT_ABS_UNARY,
  E_JMP_LS_ABS_UNARY, E_JMP_GE_ABS_UNARY, E_JMP_LE_ABS_UNARY,
};

// clang-format on

inline std::string opcode_to_string(opcode code) {
  using enum opcode;

  // clang-format off
  static const std::unordered_map<opcode, std::string> lookup_table = {
      {E_RETURN_NULLARY, "ret"}, {E_PUSH_CONST_UNARY, "push_const"},
      {E_POP_NULLARY, "pop"}, {E_ADD_NULLARY, "add"}, 
      {E_SUB_NULLARY, "sub"}, {E_MUL_NULLARY, "mul"},
      {E_DIV_NULLARY, "div"}, {E_MOD_NULLARY, "mod"},
      {E_CMP_NULLARY, "cmp"}, {E_JMP_EQ_ABS_UNARY, "jmp_eq"},
      {E_JMP_NE_ABS_UNARY, "jmp_ne"}, {E_JMP_GT_ABS_UNARY, "jmp_gt"},
      {E_JMP_LS_ABS_UNARY, "jmp_ls"}, {E_JMP_GE_ABS_UNARY, "jmp_ge"},
      {E_JMP_LE_ABS_UNARY, "jmp_le"}, {E_PUSH_LOCAL_UNARY, "push_local"},
      {E_MOV_LOCAL_UNARY, "mov_local"}, {E_PRINT_NULLARY, "print"},
      {E_PUSH_READ_NULLARY, "push_read"},
  };
  // clang-format on

  return lookup_table.at(code);
}

} // namespace paracl::bytecode_vm