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

template <typename T, std::input_iterator iter> std::optional<std::pair<T, iter>> read_little_endian(iter first, iter last) requires std::integral<T> || std::floating_point<T> { 
  std::array<char, sizeof(T)> raw_bytes;

  if (first == last) {
    return std::nullopt;
  }

  auto input_iter = typename decltype(raw_bytes)::iterator{};

  if constexpr (std::endian::native == std::endian::little) {
    input_iter = raw_bytes.begin();
  } else if constexpr (std::endian::native == std::endian::big) {
    input_iter = raw_bytes.rbegin();
  } else {
    return std::nullopt;
  }

  auto size = sizeof(T);
  first = algorithm::copy_while(first, last, input_iter, [&size](auto) {return size && size--; });

  return std::make_pair(std::bit_cast<T>(raw_bytes), first);
}

} // namespace paracl::utils::serialization