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

namespace paracl::frontend::ast {

class assignment_statement;
class binary_expression;
class constant_expression;
class if_statement;
class print_statement;
class read_expression;
class statement_block;
class statement_expression;
class unary_expression;
class variable_expression;
class while_statement;

class i_ast_visitor {
public:
  virtual void visit(assignment_statement &) = 0;
  virtual void visit(binary_expression &) = 0;
  virtual void visit(constant_expression &) = 0;
  virtual void visit(if_statement &) = 0;
  virtual void visit(print_statement &) = 0;
  virtual void visit(read_expression &) = 0;
  virtual void visit(statement_block &) = 0;
  virtual void visit(statement_expression &) = 0;
  virtual void visit(unary_expression &) = 0;
  virtual void visit(variable_expression &) = 0;
  virtual void visit(while_statement &) = 0;
};

} // namespace paracl::frontend::ast

#include "frontend/ast/assignment_statement.hpp"
#include "frontend/ast/binary_expression.hpp"
#include "frontend/ast/constant_expression.hpp"
#include "frontend/ast/i_ast_node.hpp"
#include "frontend/ast/if_statement.hpp"
#include "frontend/ast/print_statement.hpp"
#include "frontend/ast/read_expression.hpp"
#include "frontend/ast/statement_block.hpp"
#include "frontend/ast/statement_expression.hpp"
#include "frontend/ast/unary_expression.hpp"
#include "frontend/ast/variable_expression.hpp"
#include "frontend/ast/while_statement.hpp"