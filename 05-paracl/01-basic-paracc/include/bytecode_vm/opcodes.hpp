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
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>

#include "utils/serialization.hpp"

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

static inline std::string opcode_to_string(opcode code) {
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

template <typename... Ts> struct instruction {
  opcode            op;
  std::tuple<Ts...> attributes;
};

using nullary_instruction = instruction<>;
using unary_u32_instruction = instruction<uint32_t>;
using variant_instruction = std::variant<std::monostate, nullary_instruction, unary_u32_instruction>;

template <std::input_iterator iter> std::pair<variant_instruction, iter> decode_instruction(iter first, iter last) {
  if (first == last) return std::make_pair(std::monostate{}, first);
  opcode op = static_cast<opcode>(*first++);
  using enum opcode;

  switch (op) {

  // 1. Nullary operatorions
  case E_RETURN_NULLARY:
  case E_POP_NULLARY:
  case E_ADD_NULLARY:
  case E_SUB_NULLARY:
  case E_MUL_NULLARY:
  case E_DIV_NULLARY:
  case E_MOD_NULLARY:
  case E_PRINT_NULLARY:
  case E_PUSH_READ_NULLARY:
  case E_CMP_NULLARY: {
    return std::make_pair(nullary_instruction{op}, first);
  }

  // 2. Unary instruction with u32 argument
  case E_PUSH_CONST_UNARY:
  case E_JMP_ABS_UNARY:
  case E_JMP_EQ_ABS_UNARY:
  case E_JMP_NE_ABS_UNARY:
  case E_JMP_GT_ABS_UNARY:
  case E_JMP_LS_ABS_UNARY:
  case E_JMP_GE_ABS_UNARY:
  case E_JMP_LE_ABS_UNARY:
  case E_PUSH_LOCAL_UNARY:
  case E_MOV_LOCAL_UNARY: {
    auto [val, it] = utils::serialization::read_little_endian<uint32_t>(first, last);
    if (!val) return std::make_pair(std::monostate{}, it);
    return std::make_pair(unary_u32_instruction{op, val.value()}, it);
  }
  }
}

} // namespace paracl::bytecode_vm