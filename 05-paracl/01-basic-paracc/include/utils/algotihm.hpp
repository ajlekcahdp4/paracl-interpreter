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

#include <concepts>
#include <iterator>

namespace paracl::utils::algorithm {

template <std::input_iterator input_it, std::output_iterator<typename input_it::value_type> output_it, typename t_pred>
input_it copy_while(input_it first, input_it last, output_it o_first, t_pred pred) {
  for (; first != last; ++first) {
    if (!pred(*first)) break;
    *o_first = *first;
    ++o_first;
  }
  return first;
}

} // namespace paracl::utils::algorithm