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

#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"

#include <cassert>
#include <type_traits>

namespace paracl::frontend::ast {

enum class ast_node_type {
  E_ASSIGNMENT_STATEMENT,
  E_BINARY_EXPRESSION,
  E_CONSTANT_EXPRESSION,
  E_ERROR_NODE,
  E_FUNCTION_CALL,
  E_FUNCTION_DEFINITION,
  E_IF_STATEMENT,
  E_WHILE_STATEMENT,
  E_SUBSCRIPT,
  E_PRINT_STATEMENT,
  E_READ_EXPRESSION,
  E_STATEMENT_BLOCK,
  E_VALUE_BLOCK,
  E_UNARY_EXPRESSION,
  E_VARIABLE_EXPRESSION,
  E_RETURN_STATEMENT,
  E_FUNCTION_DEFINITION_TO_PTR_CONV,
};

namespace detail {
template <typename T> ast_node_type get_ast_node_type() = delete;
// clang-format off
template <> inline ast_node_type get_ast_node_type<assignment_statement>() { return ast_node_type::E_ASSIGNMENT_STATEMENT; }
template <> inline ast_node_type get_ast_node_type<binary_expression>() { return ast_node_type::E_BINARY_EXPRESSION; }
template <> inline ast_node_type get_ast_node_type<constant_expression>() { return ast_node_type::E_CONSTANT_EXPRESSION; }
template <> inline ast_node_type get_ast_node_type<error_node>() { return ast_node_type::E_ERROR_NODE; }
template <> inline ast_node_type get_ast_node_type<function_call>() { return ast_node_type::E_FUNCTION_CALL; }
template <> inline ast_node_type get_ast_node_type<function_definition>() { return ast_node_type::E_FUNCTION_DEFINITION; }
template <> inline ast_node_type get_ast_node_type<if_statement>() { return ast_node_type::E_IF_STATEMENT; }
template <> inline ast_node_type get_ast_node_type<while_statement>() { return ast_node_type::E_WHILE_STATEMENT; }
template <> inline ast_node_type get_ast_node_type<print_statement>() { return ast_node_type::E_PRINT_STATEMENT; }
template <> inline ast_node_type get_ast_node_type<read_expression>() { return ast_node_type::E_READ_EXPRESSION; }
template <> inline ast_node_type get_ast_node_type<statement_block>() { return ast_node_type::E_STATEMENT_BLOCK; }
template <> inline ast_node_type get_ast_node_type<value_block>() { return ast_node_type::E_VALUE_BLOCK; }
template <> inline ast_node_type get_ast_node_type<unary_expression>() { return ast_node_type::E_UNARY_EXPRESSION; }
template <> inline ast_node_type get_ast_node_type<variable_expression>() { return ast_node_type::E_VARIABLE_EXPRESSION; }
template <> inline ast_node_type get_ast_node_type<return_statement>() { return ast_node_type::E_RETURN_STATEMENT; }
template <> inline ast_node_type get_ast_node_type<function_definition_to_ptr_conv>() { return ast_node_type::E_FUNCTION_DEFINITION_TO_PTR_CONV; }
template <> inline ast_node_type get_ast_node_type<subscript>() { return ast_node_type::E_SUBSCRIPT; }
// clang-format on
} // namespace detail

constexpr auto ast_expression_types = std::array{
    ast_node_type::E_ASSIGNMENT_STATEMENT,
    ast_node_type::E_VALUE_BLOCK,
    ast_node_type::E_BINARY_EXPRESSION,
    ast_node_type::E_CONSTANT_EXPRESSION,
    ast_node_type::E_UNARY_EXPRESSION,
    ast_node_type::E_VARIABLE_EXPRESSION,
    ast_node_type::E_FUNCTION_DEFINITION_TO_PTR_CONV,
    ast_node_type::E_READ_EXPRESSION,
    ast_node_type::E_FUNCTION_CALL,
    ast_node_type::E_SUBSCRIPT,
};

inline ast_node_type identify_node(const i_ast_node &base) {
  return ezvis::visit_tuple<ast_node_type, ast::tuple_all_nodes>(
      [](auto &&node) { return detail::get_ast_node_type<std::remove_cvref_t<decltype(node)>>(); },
      base
  );
}

inline ast_node_type identify_node(const i_ast_node *base) {
  assert(base);
  return identify_node(*base);
}

} // namespace paracl::frontend::ast
