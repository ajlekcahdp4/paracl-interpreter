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

#include "function_table.hpp"
#include "usegraph.hpp"

#include "frontend/ast/ast_container.hpp"

namespace paracl::frontend {

struct functions_analytics final {
  named_function_table m_named;
  anonymous_function_table m_anonymous;
  usegraph m_usegraph;
};

} // namespace paracl::frontend