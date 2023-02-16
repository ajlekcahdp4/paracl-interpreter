#include "bytecode_vm/decl_vm.hpp"
#include "utils/serialization.hpp"

#include <cstdint>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>

namespace paracl::bytecode_vm::decl_vm {

constexpr unsigned                                magic_bytes_length = 6;
constexpr std::array<uint8_t, magic_bytes_length> header = {0xB, 0x0, 0x0, 0xB, 0xE, 0xC};

static std::vector<uint8_t> read_raw_data(std::istream &is) {
  if (!is) throw std::runtime_error("Invalid istream");
  std::vector<uint8_t> raw_data{std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>()};

  return raw_data;
}

std::optional<chunk> read_chunk(std::istream &is) {
  auto raw_bytes = read_raw_data(is);

  if (raw_bytes.size() < magic_bytes_length || !std::equal(header.begin(), header.end(), raw_bytes.begin())) {
    std::cerr << "Incorrect magic byte header\n";
    return std::nullopt;
  }

  auto first = raw_bytes.begin();
  std::advance(first, magic_bytes_length);
  auto last = raw_bytes.end();

  auto [count_constants, after_const_count_it] = utils::read_little_endian<uint32_t>(first, last);
  if (!count_constants) {
    std::cerr << "Invalid header\n";
    return std::nullopt;
  }

  auto [length_binary, after_binary_length_it] = utils::read_little_endian<uint32_t>(after_const_count_it, last);
  if (!length_binary) {
    std::cerr << "Invalid header\n";
    return std::nullopt;
  }

  if (raw_bytes.size() !=
      magic_bytes_length + sizeof(uint32_t) * 2 + count_constants.value() * sizeof(int) + length_binary.value()) {
    std::cerr << "File size does not match\n";
    return std::nullopt;
  }

  first = after_binary_length_it;
  constant_pool_type pool;
  for (uint32_t i = 0; i < count_constants.value(); ++i) {
    auto [constant, iter] = utils::read_little_endian<int>(first, last);
    first = iter;
    pool.push_back(constant.value());
  }

  binary_code_buffer_type buf;
  buf.reserve(length_binary.value());
  std::copy(first, last, std::back_inserter(buf));

  return chunk{std::move(buf), std::move(pool)};
}

void write_chunk(std::ostream &os, const chunk &ch) {
  // Write header with magic bytes.
  os.write(reinterpret_cast<const char *>(header.data()), header.size());
  std::array<uint8_t, sizeof(uint32_t)> size_buffer;

  // Write number of constants
  utils::write_little_endian<uint32_t>(ch.constants_size(), size_buffer.begin());
  os.write(reinterpret_cast<const char *>(size_buffer.data()), size_buffer.size());

  // Write length of binary code (in bytes)
  utils::write_little_endian<uint32_t>(ch.binary_size(), size_buffer.begin());
  os.write(reinterpret_cast<const char *>(size_buffer.data()), size_buffer.size());

  // Write constants
  std::vector<uint8_t> raw_constants;
  raw_constants.reserve(ch.constants_size() * sizeof(int));

  for (auto start = ch.constants_begin(), finish = ch.constants_end(); start != finish; ++start) {
    utils::write_little_endian(*start, std::back_inserter(raw_constants));
  }

  os.write(reinterpret_cast<const char *>(raw_constants.data()), raw_constants.size());
  os.write(reinterpret_cast<const char *>(ch.binary_data()), ch.binary_size());
}

} // namespace paracl::bytecode_vm::decl_vm