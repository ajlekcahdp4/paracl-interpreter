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
  E_DIV_NULLARY, E_MOD_NULLARY, E_AND_NULLARY, E_OR_NULLARY,
  E_CMP_EQ_NULLARY, E_CMP_NE_NULLARY, E_CMP_GT_NULLARY, E_CMP_LS_NULLARY,
  E_CMP_GE_NULLARY, E_CMP_LE_NULLARY,

  E_PRINT_NULLARY, E_PUSH_READ_NULLARY, E_PUSH_LOCAL_UNARY, E_MOV_LOCAL_UNARY,

  E_JMP_UNARY, E_JMP_FALSE_UNARY, E_JMP_TRUE_UNARY,
};
// clang-format on

} // namespace paracl::bytecode_vm