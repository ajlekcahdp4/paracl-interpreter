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

#include "ezvis/ezvis.hpp"
#include "location.hpp"

namespace paracl::frontend::ast {

class i_ast_node : public ezvis::visitable_base<i_ast_node> {
protected:
  location m_loc;

public:
  EZVIS_VISITABLE();
  location loc() { return m_loc; }

  i_ast_node() = default;
  i_ast_node(location l) : m_loc{l} {}
  virtual ~i_ast_node() {}
};

} // namespace paracl::frontend::ast