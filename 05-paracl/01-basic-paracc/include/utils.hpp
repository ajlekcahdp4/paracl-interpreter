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
#include <iomanip>

#include "utils/algotihm.hpp"
#include "utils/serialization.hpp"

namespace paracl::utils {

template <class... Ts> struct visitors : Ts... { using Ts::operator()...; };
template <class... Ts> visitors(Ts...) -> visitors<Ts...>;

}