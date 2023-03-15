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

#include "utils/algorithm.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <optional>
#include <span>
#include <sstream>

namespace paracl::utils {

namespace detail {

inline auto get_iterator_endian(std::span<char> raw_bytes) {
  static_assert(
      std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed endian, bailing out"
  );
  if constexpr (std::endian::native == std::endian::little) {
    return raw_bytes.begin();
  } else {
    return raw_bytes.rbegin();
  }
}

} // namespace detail

template <typename T, std::input_iterator iter>
std::pair<std::optional<T>, iter> read_little_endian(iter first, iter last)
  requires std::integral<T> || std::floating_point<T>
{
  std::array<char, sizeof(T)> raw_bytes;

  const auto input_iter = detail::get_iterator_endian(raw_bytes);
  auto size = sizeof(T);
  first = copy_while(first, last, input_iter, [&size](auto) { return size && size--; });

  if (size != 0) return std::make_pair(std::nullopt, first);
  return std::make_pair(std::bit_cast<T>(raw_bytes), first);
}

template <typename T, std::output_iterator<uint8_t> iter>
void write_little_endian(T val, iter oput)
  requires std::integral<T> || std::floating_point<T>
{
  std::array<char, sizeof(T)> raw_bytes = std::bit_cast<decltype(raw_bytes)>(val);

  const auto input_iter = detail::get_iterator_endian(raw_bytes);
  auto size = sizeof(T);
  std::copy_n(input_iter, size, oput);
}

struct padded_hex {
  template <typename T, typename t_stream>
  t_stream &operator()(t_stream &os, T val, unsigned padding = 8, char fill = '0') const {
    os << "0x" << std::setfill(fill) << std::setw(padding) << std::hex << val;
    return os;
  }
};

constexpr auto padded_hex_printer = padded_hex{};

template <typename T> auto pointer_to_uintptr(T *pointer) {
  return std::bit_cast<uintptr_t>(pointer);
}

inline std::string read_file(const std::filesystem::path &input_path) {
  std::ifstream ifs;
  ifs.exceptions(ifs.exceptions() | std::ios::failbit | std::ios::badbit);
  ifs.open(input_path, std::ios::binary);

  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

} // namespace paracl::utils