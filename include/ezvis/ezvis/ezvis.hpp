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

#include <ctti/type_id.hpp>

#include <cstdint>
#include <type_traits>

namespace ezvis {

namespace detail {

using unique_tag_type = uint64_t;

// This is a dummy type to get a unique type name
template <typename t_base, typename t_visitable> struct dummy_tag {};
template <typename t_base, typename t_visitable> constexpr unique_tag_type unique_tag() {
  using dummy_type = dummy_tag<std::remove_cv<t_base>, std::remove_cv<t_visitable>>;
  return ctti::unnamed_type_id<dummy_type>().hash();
}

}; // namespace detail

template <typename t_base> struct visitable_base {
  using base_type = t_base;

  template <typename t_visitable>
  static detail::unique_tag_type
  unique_tag_helper_ezviz__(const t_visitable * /* Dummy parameter for template deduction */) {
    return detail::unique_tag<base_type, t_visitable>();
  }

  virtual ezvis::detail::unique_tag_type unique_tag_ezvis__() const = 0;

#define EZVIS_VISITABLE()                                                                                              \
  virtual ezvis::detail::unique_tag_type unique_tag_ezvis__() const override { return unique_tag_helper_ezviz__(this); }
};

}; // namespace ezvis