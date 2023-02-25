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

class function_call : public i_ast_node {
private:
  i_ast_node *m_params;
  std::string m_name;

public:
  EZVIS_VISITABLE();

  function_call() = default;

  function_call(std::string name, i_ast_node &params, location l)
      : i_ast_node{l}, m_params{&params}, m_name{std::move(name)} {}

  std::string_view name() const { return m_name; }

  i_ast_node &params() const { return *m_params; }
};

class function_call_params : public i_ast_node {
private:
  std::vector<i_ast_node *> m_params;

public:
  EZVIS_VISITABLE();

  function_call_params() = default;

  function_call_params(location l, std::vector<i_ast_node *> vec = {}) : i_ast_node{l}, m_params{std::move(vec)} {}

  void append_param(i_ast_node &expr) {
    const bool empty = m_params.empty();
    m_params.push_back(&expr);

    if (empty) m_loc = expr.loc();
    else m_loc += expr.loc();
  }

  auto size() const { return m_params.size(); }

  auto begin() const { return m_params.begin(); }

  auto end() const { return m_params.end(); }
};

} // namespace paracl::frontend::ast