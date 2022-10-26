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
  E_RETURN_NULLARY = 0x00,

  E_PUSH_CONST_UNARY = 0x01,
  E_POP_NULLARY = 0x02,

  E_ADD_NULLARY = 0x03, E_SUB_NULLARY = 0x04, E_MUL_NULLARY = 0x05,
  E_DIV_NULLARY = 0x06, E_MOD_NULLARY = 0x07, E_PRINT_NULLARY = 0x08,
  E_PUSH_READ_NULLARY = 0x09, E_PUSH_LOCAL_UNARY = 0x0a, E_MOV_LOCAL_UNARY = 0x0b,

  E_CMP_NULLARY = 0x0c, E_JMP_ABS_UNARY = 0x0d,
  E_JMP_EQ_ABS_UNARY = 0x0e, E_JMP_NE_ABS_UNARY = 0x0f, E_JMP_GT_ABS_UNARY = 0x10,
  E_JMP_LS_ABS_UNARY = 0x11, E_JMP_GE_ABS_UNARY = 0x12, E_JMP_LE_ABS_UNARY = 0x13,
};
// clang-format on

} // namespace paracl::bytecode_vm