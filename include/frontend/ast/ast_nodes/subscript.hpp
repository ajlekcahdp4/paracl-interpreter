/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <alex.rom23@mail.ru> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy us a beer in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/types/types.hpp"
#include "i_ast_node.hpp"

#include <string>
#include <string_view>

namespace paracl::frontend::ast {

class subscript : public i_expression {
private:
  std::string m_name;
  i_expression *m_sub;

private:
  EZVIS_VISITABLE();

public:
  subscript(std::string name, i_expression *sub, location l)
      : i_expression{l, types::type_builtin::type_int}, m_name{std::move(name)}, m_sub(sub) {}

  std::string_view name() const & { return m_name; }

  auto get_subscript() const { return m_sub; }
};

} // namespace paracl::frontend::ast
