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
  location loc() const { return m_loc; }

  i_ast_node() = default;
  i_ast_node(location l) : m_loc{l} {}
  virtual ~i_ast_node() {}
};

class i_ast_node;
class assignment_statement;
class binary_expression;
class constant_expression;
class if_statement;
class print_statement;
class read_expression;
class statement_block;
class unary_expression;
class variable_expression;
class while_statement;
class error_node;

using tuple_ast_nodes = std::tuple<
    assignment_statement, binary_expression, constant_expression, if_statement, print_statement, read_expression,
    statement_block, unary_expression, variable_expression, while_statement, error_node>;

} // namespace paracl::frontend::ast