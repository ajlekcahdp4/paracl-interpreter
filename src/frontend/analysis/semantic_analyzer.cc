/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/analysis/semantic_analyzer.hpp"

#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"

#include "utils/misc.hpp"

#include <fmt/core.h>

#include <iostream>
#include <string>

namespace paracl::frontend {

using types::type_builtin;

void semantic_analyzer::analyze_node(ast::unary_expression &ref) {
  apply(ref.expr());
  if (expect_type_eq_cond(ref.expr(), type_builtin::type_int().base(), !m_first_recursive_traversal)) {
    ref.type = type_builtin::type_int();
  }
}

void semantic_analyzer::analyze_node(ast::error_node &ref) {
  report_error(ref.error_msg(), ref.loc());
}

void semantic_analyzer::analyze_node(ast::assignment_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.right());

  set_state(semantic_analysis_state::E_LVALUE);
  auto &&right_type = ref.right().type;
  if (!right_type) {
    report_error("Type of the right side of the assignment is unknown or can't be deduced", ref.right().loc());
    reset_state();
    return;
  }

  for (auto &v : ref) {
    auto &&declared = analyze_node(v);
    if (right_type && !declared && !v.type) {
      v.type = right_type;
    } else {
      expect_type_eq_cond(v, ref.right().type.base(), !m_first_recursive_traversal);
    }
  }

  reset_state();
  ref.type = ref.right().type;
}

void semantic_analyzer::analyze_node(ast::binary_expression &ref) {
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.right());
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.left());

  if (expect_type_eq_cond(ref.right(), type_builtin::type_int().base(), !m_first_recursive_traversal) &&
      expect_type_eq_cond(ref.left(), type_builtin::type_int().base(), !m_first_recursive_traversal)) {
    ref.type = type_builtin::type_int();
  }

  reset_state();
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);

  apply(ref.expr());
  expect_type_eq_cond(ref.expr(), type_builtin::type_int().base(), !m_first_recursive_traversal);

  reset_state();
}

using expressions_and_base = utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node>;

void semantic_analyzer::check_return_types_matches(ast::statement_block &ref) {
  auto first_type = m_return_statements.front()->type;
  auto ret_type = ref.type;
  bool valid = true;

  for (const auto &ret : m_return_statements) {
    if (ret_type) // If return type is set
      if (ret->type == ret_type) continue;
    if (ret->type == first_type) continue;

    error_report error = {
        {fmt::format("Return type deduction failed, found mismatch"), ret->loc()}
    };

    report_error(error);
    valid = false;
    break;
  }

  if (valid) ref.type = first_type;
}

void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  m_scopes.begin_scope(ref.symbol_table());

  bool is_rvalue = (current_state == semantic_analysis_state::E_RVALUE);
  // Save state, because it will get mangled by subsequent apply calls.
  auto size = ref.size();
  unsigned i = 0;
  m_return_statements.clear();
  for (auto &&statement : ref) {
    assert(statement);
    apply(*statement);
    if ((i == size - 1) && !m_in_void_block) {
      set_state(semantic_analysis_state::E_RVALUE);
      auto type = ezvis::visit_tuple<types::generic_type, expressions_and_base>(
          paracl::utils::visitors{
              [](ast::i_expression &expr) { return expr.type; },
              [&](ast::i_ast_node &) { return type_builtin::type_void(); }},
          *statement
      );
      ref.type = type;
    } else set_state(semantic_analysis_state::E_LVALUE);
    ++i;
  }

  if (!m_return_statements.empty() && !m_in_void_block) {
    check_return_types_matches(ref);
  }

  if (m_in_void_block || (!is_rvalue && !ref.type)) ref.type = type_builtin::type_void();
  ;

  m_return_statements.clear();
  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  m_scopes.begin_scope(ref.control_block_symtab());
  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int().base());

  auto block_state = m_in_void_block;
  m_in_void_block = true;
  m_scopes.begin_scope(ref.true_symtab());
  apply(ref.true_block());
  m_scopes.end_scope();
  m_in_void_block = block_state;

  if (ref.else_block() != nullptr) {
    m_scopes.begin_scope(ref.else_symtab());
    apply(*ref.else_block());
    m_scopes.end_scope();
  }

  m_scopes.end_scope();
}

void semantic_analyzer::analyze_node(ast::while_statement &ref) {
  m_scopes.begin_scope(ref.symbol_table());

  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int().base());

  auto block_state = m_in_void_block;
  m_in_void_block = true;
  apply(ref.block());
  m_in_void_block = block_state;
  m_scopes.end_scope();
}

bool semantic_analyzer::analyze_node(ast::variable_expression &ref) {
  auto attr = m_scopes.lookup_symbol(ref.name());

  if (!attr) { // Not found.
    if (current_state == semantic_analysis_state::E_LVALUE) {
      m_scopes.declare(ref.name(), &ref);
      return false;
    }

    report_error(fmt::format("Use of undeclared variable `{}`", ref.name()), ref.loc());
    return false;
  }

  assert(attr->m_definition && "Broken definition pointer");
  auto type = ezvis::visit<types::generic_type, ast::variable_expression>(
      [](ast::variable_expression &v) { return v.type; }, *attr->m_definition
  );
  ref.type = type;

  return true;
}

void semantic_analyzer::set_function_argument_types(ast::function_definition &ref) {
  std::vector<types::generic_type> m_arg_type_vec;
  std::transform(ref.begin(), ref.end(), std::back_inserter(m_arg_type_vec), [&](auto &&v) {
    if (!v.type) v.type = type_builtin::type_int(); // default argument type: int
    return v.type;
  });
  ref.type.set_argument_types(m_arg_type_vec);
}

void semantic_analyzer::begin_function_scope(ast::function_definition &ref) {
  auto *param_symtab = ref.param_symtab();
  assert(param_symtab && "Broken param symtab pointer");
  for (auto &def : ref) {
    param_symtab->declare(def.name(), &def);
  }
  m_scopes.begin_scope(param_symtab);
}

using expressions_and_return =
    utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node, ast::return_statement>;

void semantic_analyzer::analyze_node(ast::function_definition &ref) {
  if (m_in_function_body) return;
  m_in_function_body = true; // Set flag

  begin_function_scope(ref);
  set_function_argument_types(ref);

  auto &&body = ref.body();
  auto body_type = ast::identify_node(body);
  assert(
      body_type == ast::ast_node_type::E_ERROR_NODE ||
      body_type == ast::ast_node_type::E_STATEMENT_BLOCK && "Broken statement block pointer"
  );
  if (body_type == ast::ast_node_type::E_ERROR_NODE) {
    apply(body);                // Visit error node
    m_in_function_body = false; // Exit
    m_scopes.end_scope();
    return;
  }

  // The only other possibility for the reference is statement block.
  auto &&st_block = static_cast<ast::statement_block &>(body);

  // "The only difference between function and statement blocks in paraCL
  // is that we can call functions several times"
  apply(st_block);
  ref.type.m_return_type = st_block.type;
  m_scopes.end_scope();

  m_in_function_body = false; // Exit
}

void semantic_analyzer::analyze_node(ast::function_definition_to_ptr_conv &ref) {
  ref.type = types::generic_type::make<types::type_composite_function>(ref.definition().type);
}

void semantic_analyzer::analyze_node(ast::function_call &ref) {
  for (auto &&param : ref) {
    apply(*param);
  }

  auto &&name = ref.name();
  auto &&function_found = m_functions->m_named.lookup(std::string{name});
  auto &&attr = m_scopes.lookup_symbol(name);

  const auto report = [&](auto &&loc) {
    error_report error = {
        {fmt::format("Call parameter type/count mismatch", name), ref.loc()}
    };

    error.add_attachment({fmt::format("Defined here", name), loc});
    report_error(error);
  };

  const auto match_expr_type = [&](auto *expr_ptr, auto &&arg) {
    return expect_type_eq_cond(*expr_ptr, arg.type.base(), !m_first_recursive_traversal);
  };

  const auto match_types = [&](auto *expr_ptr, auto &&arg) {
    return expect_type_eq_cond(*expr_ptr, arg.base(), !m_first_recursive_traversal);
  };

  const auto check_func_parameter_list = [&](auto &&type, auto &&loc, auto match) {
    if (std::mismatch(ref.begin(), ref.end(), type.cbegin(), type.cend(), match).first != ref.end() ||
        ref.size() != type.size()) {
      report(loc);
      return false;
    }
    return true;
  };

  if (function_found) {
    if (attr) {
      error_report report = {
          {fmt::format("Ambiguous call of `{}`", name), ref.loc()}
      };
      report.add_attachment({fmt::format("[Info] Have variable `{}`", name), attr->m_definition->loc()});
      report.add_attachment({fmt::format("[Info] Have function `{}`", name), function_found->loc()});
      report_error(report);
      return;
    } else {
      if (check_func_parameter_list(*function_found, function_found->loc(), match_expr_type))
        ref.type = function_found->type.return_type();
      return;
    }
  }
  if (attr) {
    auto *def = attr->m_definition;
    auto &&type = def->type;
    if (type.base().get_class() != types::type_class::E_COMPOSITE_FUNCTION) {
      report(def->loc());
      return;
    }
    auto &&cast_type = static_cast<types::type_composite_function &>(def->type.base());
    if (check_func_parameter_list(cast_type, def->loc(), match_types)) ref.type = cast_type.return_type();
    return;
  }

  error_report error = {
      {fmt::format("Unknown function to call `{}`", name), ref.loc()}
  };

  report_error(error);
}

void semantic_analyzer::analyze_node(ast::return_statement &ref) {
  if (!ref.empty()) apply(ref.expr());
  m_return_statements.push_back(&ref);
  ref.type = ref.expr().type;
}

} // namespace paracl::frontend