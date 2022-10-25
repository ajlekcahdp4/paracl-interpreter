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

#include <vector>

#include "bytecode_vm/constant_pool.hpp"
#include "bytecode_vm/opcodes.hpp"

#include "utils/serialization.hpp"

#include <array>
#include <bit>
#include <concepts>
#include <iostream>
#include <string>

namespace paracl::bytecode_vm {

using binary_code_buffer = std::vector<uint8_t>;

class chunk {
public:
  binary_code_buffer m_binary_code;
  constant_pool      m_constant_pool;

  chunk() = default;

  chunk(std::vector<uint8_t> &&p_bin, constant_pool &&p_const)
      : m_binary_code{std::move(p_bin)}, m_constant_pool{std::move(p_const)} {}

  template <std::input_iterator binary_it, std::input_iterator constant_it>
  chunk(binary_it bin_begin, binary_it bin_end, constant_it const_begin, constant_it const_end)
      : m_binary_code{bin_begin, bin_end}, m_constant_pool{const_begin, const_end} {}

  void push_opcode(opcode op) { push_byte(static_cast<uint8_t>(op)); }
  void push_byte(uint8_t code) { m_binary_code.push_back(code); }

  template <typename T> void push_value(T val) {
    utils::serialization::write_little_endian(val, std::back_inserter(m_binary_code));
  };

  void push_signed_byte(int8_t val) { m_binary_code.push_back(std::bit_cast<uint8_t>(val)); }
};

std::optional<chunk> read_chunk(std::istream &);
void  write_chunk(std::ostream &, const chunk &);

} // namespace paracl::bytecode_vm