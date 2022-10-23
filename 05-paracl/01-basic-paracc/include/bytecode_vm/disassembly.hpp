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
      padded_hex_printer(os, i) << " = { " << std::dec << pool[i] << " }\n";
    }

    return os;
  }
};

template <class... Ts> struct visitors : Ts... { using Ts::operator()...; };
template <class... Ts> visitors(Ts...) -> visitors<Ts...>;

class chunk_binary_disassembler {
private:
  template <typename t_stream>
  std::optional<binary_code_buffer::const_iterator> operator()(t_stream &os, const chunk &chk, auto first,
                                                               auto last) const {
    if (first == last) {
      os << "<Unexpectedly reached the end of range>\n";
      return std::nullopt;
    }

    auto [instr, it] = decode_instruction(first, last);
    const auto &pool = chk.m_constant_pool;
    using enum opcode;

    auto visitor = visitors{[&os](const nullary_instruction &nullary) { os << opcode_to_string(nullary.op); },
                            [&os, &pool](const unary_u32_instruction &unary) {
                              auto first_attr = std::get<0>(unary.attributes);

                              switch (unary.op) {
                              case E_PUSH_CONST_UNARY: {
                                os << opcode_to_string(unary.op) << " [ " << std::dec << pool.at(first_attr) << " ]";
                                break;
                              }

                              case E_PUSH_LOCAL_UNARY:
                              case E_MOV_LOCAL_UNARY:
                              case E_JMP_ABS_UNARY:
                              case E_JMP_EQ_ABS_UNARY:
                              case E_JMP_NE_ABS_UNARY:
                              case E_JMP_GT_ABS_UNARY:
                              case E_JMP_GE_ABS_UNARY:
                              case E_JMP_LE_ABS_UNARY: {
                                os << opcode_to_string(unary.op) << " [ ";
                                padded_hex_printer(os, first_attr) << " ] ";
                                break;
                              }

                              default: throw std::runtime_error{"Unexpected error encountered"};
                              }
                            },
                            [&os](std::monostate) { std::cout << "<Incorrectly encoded instruction>"; }};

    std::visit(visitor, instr);

    return it;
  } // namespace paracl::bytecode_vm::disassembly

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