/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "i_ast_node.hpp"

#include <string>
#include <string_view>

namespace paracl::frontend::ast {

class function_call : public i_ast_node, private std::vector<i_ast_node *> {
private:
  std::string m_name;

public:
  EZVIS_VISITABLE();

  function_call(std::string name, location l, std::vector<i_ast_node *> params = {})
      : i_ast_node{l}, vector{std::move(params)}, m_name{std::move(name)} {}

  std::string_view name() const & { return m_name; }

  using vector::cbegin;
  using vector::cend;
  using vector::crbegin;
  using vector::crend;
  using vector::empty;
  using vector::size;

  auto begin() const { return vector::begin(); }
  auto end() const { return vector::end(); }

  void append_parameter(i_ast_node *ptr) { vector::push_back(ptr); }
};

} // namespace paracl::frontend::ast