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
#include <bit>
#include <iterator>
#include <array>
#include <concepts>
#include <optional>

#include "utils/algotihm.hpp"

namespace paracl::utils::serialization {

template <typename T, std::input_iterator iter> std::pair<std::optional<T>, iter> read_little_endian(iter first, iter last) requires std::integral<T> || std::floating_point<T> { 
  std::array<char, sizeof(T)> raw_bytes;

  auto input_iter = typename decltype(raw_bytes)::iterator{};

  if constexpr (std::endian::native == std::endian::little) {
    input_iter = raw_bytes.begin();
  } else if constexpr (std::endian::native == std::endian::big) {
    input_iter = raw_bytes.rbegin();
  } else {
    return std::make_pair(std::nullopt, first);
  }

  auto size = sizeof(T);
  first = algorithm::copy_while(first, last, input_iter, [&size](auto) {return size && size--; });

  if (size != 0) return std::make_pair(std::nullopt, first);
  return std::make_pair(std::bit_cast<T>(raw_bytes), first);
}

template <typename T, std::output_iterator<uint8_t> iter> void write_little_endian(T val, iter oput) requires std::integral<T> || std::floating_point<T> { 
  std::array<char, sizeof(T)> raw_bytes = std::bit_cast<decltype(raw_bytes)>(val);

  auto input_iter = typename decltype(raw_bytes)::iterator{};

  if constexpr (std::endian::native == std::endian::little) {
    input_iter = raw_bytes.begin();
  } else if constexpr (std::endian::native == std::endian::big) {
    input_iter = raw_bytes.rbegin();
  } else {
    throw std::runtime_error{"Mixed endian, don't know what to do"};
  }

  auto size = sizeof(T);
  std::copy_n(input_iter, size, oput);
}

} // namespace paracl::utils::serialization