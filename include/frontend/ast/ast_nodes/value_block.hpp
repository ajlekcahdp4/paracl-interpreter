/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "statement_block.hpp"

namespace paracl::frontend::ast {

class value_block : public i_expression {
  EZVIS_VISITABLE();

public:
  statement_block st_block;
};

} // namespace paracl::frontend::ast