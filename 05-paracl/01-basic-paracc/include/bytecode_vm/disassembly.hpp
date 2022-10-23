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

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <iomanip>
#include <iterator>
#include <optional>

#include "bytecode_vm/chunk.hpp"
#include "bytecode_vm/constant_pool.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm::disassembly {

struct padded_hex {
  template <typename T, typename t_stream>
  t_stream &operator()(t_stream &os, T val, unsigned padding = 8, char fill = '0') const {
    os << "0x" << std::setfill(fill) << std::setw(padding) << std::hex << val;
    return os;
  }
};

constexpr auto padded_hex_printer = padded_hex{};

class constant_pool_disassembler {
public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const constant_pool &pool) const {
    os << ".constant_pool\n";

    for (constant_pool::size_type i = 0; i < pool.size(); ++i) {
      padded_hex_printer(os, i) << " " << std::dec << pool[i] << "\n";
    }

    return os;
  }
};

class chunk_binary_disassembler {
private:
  template <typename t_stream>
  std::optional<binary_code_buffer::const_iterator> operator()(t_stream &os, const chunk &chk, auto first, auto last) const {
    if (first == last) {
      os << "<Unexpectedly reached the end of range>\n";
      return std::nullopt;
    }

    auto        op = static_cast<opcode>(*first++);
    const auto &pool = chk.m_constant_pool;

    using enum opcode;

    switch (op) {

    case E_RETURN_NULLARY:
    case E_POP_NULLARY:
    case E_ADD_NULLARY:
    case E_SUB_NULLARY:
    case E_MUL_NULLARY:
    case E_DIV_NULLARY:
    case E_MOD_NULLARY:
    case E_PRINT_NULLARY:
    case E_PUSH_READ_NULLARY: // Intentional fallthrough.
    case E_CMP_NULLARY: os << opcode_to_string(op); break;

    case E_PUSH_CONST_UNARY: {
      auto read_val = utils::serialization::read_little_endian<uint32_t>(first, last);

      if (!read_val) {
        os << "<Incorrectly encoded push_const>\n";
        return std::nullopt;
      }

      auto [val, new_iter] = read_val.value();
      os << opcode_to_string(op) << " [ " << std::dec << pool.at(val) << " ]";
      return new_iter;
    }

    case E_JMP_ABS_UNARY:
    case E_JMP_EQ_ABS_UNARY:
    case E_JMP_NE_ABS_UNARY:
    case E_JMP_GT_ABS_UNARY:
    case E_JMP_LS_ABS_UNARY:
    case E_JMP_GE_ABS_UNARY:
    case E_JMP_LE_ABS_UNARY:
    case E_PUSH_LOCAL_UNARY:
    case E_MOV_LOCAL_UNARY: {
      auto read_val = utils::serialization::read_little_endian<uint32_t>(first, last);
      auto instr_name = opcode_to_string(op);

      if (!read_val) {
        os << "<Incorrectly encoded " << instr_name << ">\n";
        return std::nullopt;
      }

      auto [addr, new_iter] = read_val.value();

      os << instr_name << " [ ";
      padded_hex_printer(os, addr) << " ]";

      return new_iter;
    }

    default: {
      os << "<Unknown opcode>\n";
      return std::nullopt;
    }
    }

    return first;
  }

public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    os << ".code\n";

    const auto &binary = chk.m_binary_code;
    auto        start = binary.begin();

    for (auto first = binary.begin(), last = binary.end(); first != last;) {
      padded_hex_printer(os, std::distance(start, first)) << " ";
      auto disas_instr = operator()(os, chk, first, last);
      if (!disas_instr) {
        break;
      }

      first = disas_instr.value();
      os << "\n";
    }

    return os;
  }
};

class chunk_complete_disassembler {
public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    constant_pool_disassembler{}(os, chk.m_constant_pool);
    os << "\n";
    chunk_binary_disassembler{}(os, chk);
    return os;
  }
};

} // namespace paracl::bytecode_vm::disassembly