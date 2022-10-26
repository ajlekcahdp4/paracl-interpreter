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

namespace paracl::bytecode_vm {

// clang-format off
enum opcode : std::uint8_t {
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

} // namespace paracl::bytecode_vm