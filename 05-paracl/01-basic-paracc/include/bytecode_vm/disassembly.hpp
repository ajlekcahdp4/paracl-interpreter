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
#include <stdexcept>

#include "bytecode_vm/decl_vm.hpp"
#include "utils.hpp"

namespace paracl::bytecode_vm::decl_vm::disassembly {

class constant_pool_disassembler {
public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const constant_pool &pool) const {
    os << ".constant_pool\n";

    for (constant_pool::size_type i = 0; i < pool.size(); ++i) {
      utils::serialization::padded_hex_printer(os, i) << " = { " << std::dec << pool[i] << " }\n";
    }

    return os;
  }
};

template <typename t_instr_set> class chunk_binary_disassembler {
public:
  t_instr_set instruction_set;
  chunk_binary_disassembler(const t_instr_set &isa) : instruction_set{isa} {}

public:
  void operator()(auto &os, auto &first, auto last) const {
    if (first == last) {
      throw std::runtime_error{"Unexpectedly reached the end of range"};
    }

    auto current_instruction = instruction_set.instruction_lookup_table[*first++];
    // clang-format off
    std::visit(paracl::utils::visitors{
      [&](std::monostate) {
        throw std::runtime_error{"Unknown opcode"};},
      [&](auto&& instr) {
        auto attr = instr->decode(first, last).attributes;
        instr->pretty_print(os, attr); }},
      current_instruction);
    // clang-format on
  }

public:
  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    os << ".code\n";

    const auto &binary = chk.m_binary_code;
    auto        start = binary.begin();

    for (auto first = binary.begin(), last = binary.end(); first != last;) {
      utils::serialization::padded_hex_printer(os, std::distance(start, first)) << " ";
      operator()(os, first, last);
      os << "\n";
    }

    return os;
  }
};

template <typename t_instr_set> class chunk_complete_disassembler {
  chunk_binary_disassembler<t_instr_set> binary_disas;

public:
  chunk_complete_disassembler(t_instr_set isa) : binary_disas{isa} {}

  template <typename t_stream> t_stream &operator()(t_stream &os, const chunk &chk) const {
    constant_pool_disassembler{}(os, chk.m_constant_pool);
    os << "\n";
    binary_disas(os, chk);
    return os;
  }
};

} // namespace paracl::bytecode_vm::decl_vm::disassembly