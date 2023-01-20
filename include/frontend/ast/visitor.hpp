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

class i_ast_visitor {
public:
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

#include "frontend/ast/ast_nodes.hpp"

namespace paracl::frontend::ast {
static inline void ast_node_visit(i_ast_visitor &visitor, i_ast_node *ast_node) { ast_node->accept(visitor); }

} // namespace paracl::frontend::ast