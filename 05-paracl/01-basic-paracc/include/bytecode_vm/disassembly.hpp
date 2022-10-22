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

class constant_pool_disassembler {
public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const constant_pool &pool) const {
    os << ".constant_pool\n";

    for (constant_pool::size_type i = 0; i < pool.size(); ++i) {
      os << "0x" << std::setfill('0') << std::setw(8) << std::hex << i << " " << std::dec << pool[i] << "\n";
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
    const auto &pool = chk.const_pool();

    using enum opcode;

    switch (op) {

    case E_RETURN_NULLARY:
    case E_POP_NULLARY:
    case E_ADD_NULLARY:
    case E_SUB_NULLARY:
    case E_MUL_NULLARY:
    case E_DIV_NULLARY:
    case E_MOD_NULLARY: // Intentional fallthrough.
    case E_CMP_NULLARY: os << opcode_to_string(op); break;

    case E_PUSH_CONST_UNARY: {
      auto [val, new_iter] = utils::serialization::read_little_endian<unsigned>(first, last).value();
      os << opcode_to_string(op) << " [ " << std::dec << pool.at(val) << " ]";
      return new_iter;
    }


    }

    return first;
  }

public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    os << ".code\n";

    const auto &binary = chk.binary_code();
    auto start = binary.begin();
    
    for (auto first = binary.begin(), last = binary.end(); first != last; ) {
      os << "0x" << std::setfill('0') << std::setw(8) << std::hex << std::distance(start, first) << " ";
      first = operator()(os, chk, first, last).value();
      os << "\n";
    }

    return os;
  }
};

class chunk_complete_disassembler {
public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    constant_pool_disassembler{}(os, chk.const_pool());
    os << "\n";
    chunk_binary_disassembler{}(os, chk);
    return os;
  }
};

} // namespace paracl::bytecode_vm::disassembly