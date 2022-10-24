#include "bytecode_vm/chunk.hpp"
#include "utils/serialization.hpp"
#include "bytecode_vm/disassembly.hpp"

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

void write_chunk(std::ostream &os, const chunk &ch) {
  constexpr unsigned magic_byte_length = 6;
  std::array<uint8_t, magic_byte_length> header = {0xB, 0x0, 0x0, 0xB, 0xE, 0xC};
  
  // Write header with magic bytes.
  os.write(reinterpret_cast<char *>(header.data()), header.size());
  std::array<uint8_t, sizeof(uint32_t)> size_buffer;

  utils::serialization::write_little_endian(ch.m_constant_pool.size(), size_buffer.begin());
  os.write(reinterpret_cast<char *>(size_buffer.data()), size_buffer.size());
  utils::serialization::write_little_endian(ch.m_binary_code.size(), size_buffer.begin());
  os.write(reinterpret_cast<char *>(size_buffer.data()), size_buffer.size());

  os.write(reinterpret_cast<const char *>(ch.m_constant_pool.data()), ch.m_constant_pool.size() * sizeof(int));
  os.write(reinterpret_cast<const char *>(ch.m_binary_code.data()), ch.m_binary_code.size());
}

} // namespace paracl::bytecode_vm