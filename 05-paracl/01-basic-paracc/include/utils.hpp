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
#include <variant>

#include "utils/algotihm.hpp"
#include "utils/serialization.hpp"

namespace paracl::utils {

template <class... Ts> struct visitors : Ts... { using Ts::operator()...; };
template <class... Ts> visitors(Ts...) -> visitors<Ts...>;

template <typename t_tuple> struct variant_from_tuple;
template <typename... Ts> struct variant_from_tuple<std::tuple<Ts...>> { using type = std::variant<Ts...>; };

template <typename t_tuple> using variant_from_tuple_t = typename variant_from_tuple<t_tuple>::type;

} // namespace paracl::utils