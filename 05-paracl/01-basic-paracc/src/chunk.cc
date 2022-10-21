#include "bytecode_vm/chunk.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace paracl::bytecode_vm {

static std::vector<uint8_t> read_raw_data(std::istream &is) {
  if (!is) throw std::runtime_error("Invalid istream");

  std::vector<uint8_t> raw_data;

  is.seekg(is.end);
  auto length = is.tellg();
  is.seekg(is.beg);

  raw_data.resize(length);
  is.read(reinterpret_cast<char *>(raw_data.data()), length); // This is ugly.

  return raw_data;
}

chunk read_chunk(std::istream &is) {}

static std::string disassemble_const_pool(const constant_pool &pool) {
  std::stringstream ss;

  ss << ".constant_pool:\n";

  for (unsigned i = 0; const auto &v : pool) {
    ss << "0x" << std::setfill('0') << std::setw(8) << std::hex << i << " " << std::dec << pool.at(i) << "\n";
    i++;
  }

  return ss.str();
}

template <std::integral T, std::input_iterator iter> std::pair<T, iter> read_little_endian(iter first, iter second) {
  char raw_bytes[sizeof(T)];

  if (std::distance(first, second) < sizeof(T)) throw std::runtime_error{"Can't read value"};

  if constexpr (std::endian::native == std::endian::little) {
    for (unsigned i = 0; i < sizeof(T); ++i) {
      raw_bytes[i] = *first++;
    }
  } else if constexpr (std::endian::native == std::endian::big) {
    for (int i = sizeof(T); i >= 0; --i) {
      raw_bytes[i] = *first++;
    }
  } else {
    throw std::runtime_error{"Mixed endian machine, don't know what to do"};
  }

  return {std::bit_cast<T>(raw_bytes), first};
}

std::string disassemble_chunk(const chunk &ch) {
  std::stringstream ss;

  const auto &pool = ch.const_pool();
  ss << disassemble_const_pool(pool);

  const auto &binary = ch.binary_code();
  auto        start = binary.begin(), finish = binary.end();

  using enum opcode;
  while (start != finish) {
    opcode op = static_cast<opcode>(*start++);

    ss << "0x" << std::setfill('0') << std::setw(8) << std::hex << std::distance(binary.begin(), start) << " ";

    switch (op) {

    case E_RETURN_NULLARY:
    case E_POP_NULLARY:
    case E_ADD_NULLARY:
    case E_SUB_NULLARY:
    case E_MUL_NULLARY:
    case E_DIV_NULLARY:
    case E_MOD_NULLARY:
    case E_CMP_NULLARY: ss << opcode_to_string(op) << "\n"; break;

    case E_PUSH_CONST_UNARY: {
      auto [val, new_iter] = read_little_endian<int>(start, finish);
      start = new_iter;
      ss << opcode_to_string(op) << " [ " << std::dec << pool.at(val) << " ]\n";
      break;
    }
    }
  }

  return ss.str();
}

} // namespace paracl::bytecode_vm