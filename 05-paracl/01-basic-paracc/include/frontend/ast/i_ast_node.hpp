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

#include <memory>

namespace paracl::frontend::ast {

class i_ast_visitor;

class i_ast_node {
public:
  virtual ~i_ast_node() {}
  virtual void accept(i_ast_visitor &) = 0;
};

using i_ast_node_uptr = std::unique_ptr<i_ast_node>;

class i_expression_node : public i_ast_node {};
using i_expression_node_uptr = std::unique_ptr<i_expression_node>;

class i_statement_node : public i_ast_node {};
using i_statement_node_uptr = std::unique_ptr<i_statement_node>;

} // namespace paracl::frontend::ast

#include "visitor/visitor.hpp"