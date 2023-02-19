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

#include <concepts>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <tuple>
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

#define EZVIS_VISITABLE()                                                                                              \
  virtual ezvis::detail::unique_tag_type unique_tag_ezvis__() const { return unique_tag_helper_ezviz__(this); }
};

namespace detail {

template <typename t_base, typename t_visitor, typename t_return_type> struct vtable_traits {
  using base_type = t_base;                                           // Base type in the hierarchy
  using visitor_type = t_visitor;                                     // Visitor base class
  using return_type = t_return_type;                                  // Type that visit method returns
  using function_type = t_return_type (visitor_type::*)(base_type &); // Pointer to member function type
};

template <typename t_traits> struct vtable {
  using base_type = typename t_traits::base_type;
  using visitor_type = typename t_traits::visitor_type;
  using return_type = typename t_traits::return_type;
  using function_type = typename t_traits::function_type;

private:
  std::map<detail::unique_tag_type, function_type> m_table;

public:
  template <typename t_visitable> void add(function_type func) {
    m_table[detail::unique_tag<base_type, t_visitable>()] = func;
  }

  function_type get(detail::unique_tag_type tag) const {
    auto found_handler = m_table.find(tag);
    if (found_handler != m_table.end()) return found_handler->second;
    auto found_base = m_table.find(detail::unique_tag<base_type, base_type>());
    if (found_base != m_table.end()) return found_base->second;
    throw std::runtime_error{"Unimplemented fallback visit"};
  }
};

template <typename t_traits, typename t_to_visit> class vtable_builder {
public:
  using vtable_type = vtable<t_traits>;
  using visitor_type = typename t_traits::visitor_type;
  using base_type = typename t_traits::base_type;
  using to_visit = t_to_visit;

private:
  vtable_type m_vtable;

  template <typename T> struct init_helper {};
  template <typename... t_types> struct init_helper<std::tuple<t_types...>> {
    static void put(vtable_type &table) {
      (table.template add<t_types>(
           &visitor_type::template thunk_ezvis__<visitor_type, t_types, typename visitor_type::invoker_ezvis__>),
       ...);
    }
  };

public:
  vtable_builder() { init_helper<to_visit>::put(m_vtable); }
  const auto *get() { return &m_vtable; }
};

template <typename t_traits, typename t_to_visit> const auto *get_vtable() {
  static vtable_builder<t_traits, t_to_visit> builder;
  return builder.get();
}

} // namespace detail

template <typename t_base, typename t_concrete_visitor, typename t_return_type> class visitor_base {
private:
  using vtable_traits_type = detail::vtable_traits<t_base, t_concrete_visitor, t_return_type>;

  using base_type = t_base;
  using return_type = t_return_type;
  using function_type = typename vtable_traits_type::function_type;

public:
  template <typename t_visitor, typename t_visitable, typename t_invoker> t_return_type thunk_ezvis__(t_base &base) {
    constexpr bool has_invoke = requires(t_visitor & visitor, t_visitable & visitable) {
                                  {
                                    t_invoker::template invoke<t_return_type>(visitor, visitable)
                                    } -> std::convertible_to<t_return_type>;
                                };
    static_assert(has_invoke, "Invoker type does not have an appropriate invoke method");
    return t_invoker::template invoke<t_return_type>(static_cast<t_visitor &>(*this), static_cast<t_visitable &>(base));
  }

public:
  return_type apply(base_type &base) {
    const auto   *vtable = static_cast<t_concrete_visitor &>(*this).template get_vtable_ezviz__<vtable_traits_type>();
    function_type thunk = vtable->get(base.unique_tag_ezvis__());
    return (static_cast<t_concrete_visitor &>(*this).*thunk)(base);
  }

  return_type operator()(base_type &base) { return apply(base); }

  // clang-format off
#define EZVIS_VISIT_INVOKER(name)                                                                                      \
  struct invoker_ezvis__ {                                                                                             \
    template <typename t_return_type, typename t_visitor, typename t_visitable>                                        \
    static t_return_type invoke(t_visitor &visitor, t_visitable &visitable) {                                          \
      return visitor. name (visitable);                                                                                \
    }                                                                                                                  \
  };
  // clang-format on

#define EZVIS_VISIT(tovisit)                                                                                           \
  template <typename t_traits> const auto *get_vtable_ezviz__() const {                                                \
    return ezvis::detail::get_vtable<t_traits, tovisit>();                                                             \
  }
};

}; // namespace ezvis