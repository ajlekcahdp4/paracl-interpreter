/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cassert>
#include <tuple>

namespace paracl::frontend::ast {

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

using tuple_ast_nodes =
    std::tuple<assignment_statement, binary_expression, constant_expression, if_statement, print_statement,
               read_expression, statement_block, unary_expression, variable_expression, while_statement, error_node>;

struct i_ast_visitor {
  virtual void visit(assignment_statement *) = 0;
  virtual void visit(binary_expression *) = 0;
  virtual void visit(constant_expression *) = 0;
  virtual void visit(if_statement *) = 0;
  virtual void visit(print_statement *) = 0;
  virtual void visit(read_expression *) = 0;
  virtual void visit(statement_block *) = 0;
  virtual void visit(unary_expression *) = 0;
  virtual void visit(variable_expression *) = 0;
  virtual void visit(while_statement *) = 0;
  virtual void visit(error_node *) = 0;

  virtual ~i_ast_visitor() {}
};

} // namespace paracl::frontend::ast

#include "ast_nodes/i_ast_node.hpp"

namespace paracl::frontend::ast {
inline void ast_node_visit(i_ast_visitor &visitor, i_ast_node *ast_node) {
  assert(ast_node);
  ast_node->accept(visitor);
}
} // namespace paracl::frontend::ast
