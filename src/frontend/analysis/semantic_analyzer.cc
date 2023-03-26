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
  if (!expect_type_eq(ref.expr(), type_builtin::type_int())) return;
  ref.type = type_builtin::type_int();
}

void semantic_analyzer::analyze_node(ast::assignment_statement &ref) {
  next_value_block();
  apply(ref.right());

  auto &right_type = ref.right().type;
  if (!right_type) {
    if (!m_type_errors_allowed) {
      report_error("Type of the right side of the assignment is unknown or can't be deduced", ref.right().loc());
    }
    return;
  }

  if (right_type == types::type_builtin::type_void()) {
    report_error("Type of the right side of the assignment can't be `void`", ref.right().loc());
    return;
  }

  for (auto &v : ref) {
    bool declared = analyze_node(v, true);
    if (right_type && !declared && !v.type) {
      v.type = right_type;
    } else {
      if (expect_type_eq(v, ref.right().type)) continue;
      return;
    }
  }

  ref.type = ref.right().type;
}

void semantic_analyzer::analyze_node(ast::binary_expression &ref) {
  apply(ref.right());
  apply(ref.left());

  if (expect_type_eq(ref.right(), type_builtin::type_int()) && expect_type_eq(ref.left(), type_builtin::type_int())) {
    ref.type = type_builtin::type_int();
  }
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  apply(ref.expr());
  expect_type_eq(ref.expr(), type_builtin::type_int());
}

void semantic_analyzer::check_return_types_matches(types::generic_type &type, location loc) {
  bool valid = true;

  const auto on_error = [&](location loc) {
    error_report error = {
        {fmt::format("Return type deduction failed, found mismatch"), loc}
    };

    report_error(error);
    valid = false;
  };

  auto &ret_type = type;
  if (m_return_statements->empty()) {
    if (!ret_type || ret_type == types::type_builtin::type_void()) {
      ret_type = types::type_builtin::type_void();
      return;
    }

    on_error(loc);
    return;
  }

  auto &first_type = m_return_statements->front()->type;

  for (const auto &st : *m_return_statements) {
    assert(st && "[Dedug]: Broken statement pointer");
    if (st->type && st->type == first_type) continue;
    on_error(st->loc());
  }

  if (valid) type = first_type;
}

using expressions_and_base = utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node>;
void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  begin_scope(ref.stab);

  auto *old_returns = m_return_statements;
  if (!in_raw_block()) {
    m_return_statements = &ref.return_statements;
    ref.return_statements.clear();
  }

  ref.type = types::type_builtin::type_void();
  for (auto start = ref.begin(), finish = ref.end(); start != finish; ++start) {
    auto ptr = *start;
    assert(ptr && "Broken statement pointer in a block");
    auto &st = *ptr;
    apply(st);

    bool is_last = (start == std::prev(finish));
    if (!is_last) continue;

    if (!in_raw_block()) {
      auto type = ezvis::visit_tuple<types::generic_type, expressions_and_base>(
          paracl::utils::visitors{
              [](ast::i_expression &expr) { return expr.type; },
              [&](ast::i_ast_node &) { return type_builtin::type_void(); }},
          st
      );

      if (!type) break;
      // Implicit return case
      bool is_implicit_return = ref.return_statements.empty() && type != types::type_builtin::type_void();
      if (is_implicit_return) {
        assert(m_ast && "Nullptr in m_ast");

        auto expr_ptr = ezvis::visit_tuple<ast::i_expression *, ast::tuple_expression_nodes>(
            [](ast::i_expression &expr) { return &expr; }, st
        );
        auto &ret = m_ast->make_node<ast::return_statement>(expr_ptr, expr_ptr->loc());

        ref.return_statements.push_back(&ret);
        *start = &ret;
      }

      ref.type = type;
    }
  }

  if (!in_raw_block()) check_return_types_matches(ref.type, ref.loc());

  m_return_statements = old_returns;

  end_scope();
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  begin_scope(ref.control_block_symtab());
  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int().base());

  next_raw_block();
  begin_scope(ref.true_symtab());
  apply(ref.true_block());
  end_scope();

  if (ref.else_block()) {
    next_raw_block();
    begin_scope(ref.else_symtab());
    apply(*ref.else_block());
    end_scope();
  }

  end_scope();
}

void semantic_analyzer::analyze_node(ast::while_statement &ref) {
  begin_scope(ref.symbol_table());

  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int());

  next_raw_block();
  apply(ref.block());

  end_scope();
}

bool semantic_analyzer::analyze_node(ast::variable_expression &ref, bool can_declare) {
  auto attr = m_scopes.lookup_symbol(ref.name());

  if (!attr) { // Not found.
    if (can_declare) {
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

using expressions_and_return =
    utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node, ast::return_statement>;

void semantic_analyzer::analyze_node(ast::function_definition &ref) {
  if (m_in_function_body) return;
  begin_scope(ref.param_stab);
  m_in_function_body = true; // Set flag

  auto block_ptr = ezvis::visit<ast::statement_block *, ast::error_node, ast::statement_block>(
      paracl::utils::visitors{
          [&](const ast::error_node &e) {
            analyze_node(e);
            return nullptr;
          },
          [](ast::statement_block &s) { return &s; }},
      ref.body()
  );

  auto &block_ref = *block_ptr;
  if (block_ptr) {
    next_value_block();
    auto *old_returns = m_return_statements;
    m_return_statements = &block_ref.return_statements;
    analyze_node(block_ref);

    ref.type.m_return_type = block_ref.type;
    block_ref.type = type_builtin::type_void();

    m_return_statements = old_returns;
  }

  end_scope();
  m_in_function_body = false; // Exit
}

void semantic_analyzer::analyze_node(ast::function_call &ref) {
  for (auto &&param : ref) {
    apply(*param);
  }

  auto name = ref.name();
  auto function_found = m_functions->named_functions.lookup(name);
  auto attr = m_scopes.lookup_symbol(name);

  const auto report = [&](auto &&loc) {
    if (m_type_errors_allowed) return;

    error_report error = {
        {fmt::format("Call parameter type/count mismatch", name), ref.loc()}
    };

    error.add_attachment({fmt::format("Defined here", name), loc});
    report_error(error);
  };

  const auto match_types = [&](auto expr_ptr, auto &&arg) { return expect_type_eq(*expr_ptr, arg.base()); };
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
      report.add_attachment({fmt::format("[Info] Have function `{}`", name), function_found->definition->loc()});
      report_error(report);
      return;
    }

    if (check_func_parameter_list(function_found->definition->type, function_found->definition->loc(), match_types)) {
      ref.type = function_found->definition->type.return_type();
    }

    return;
  }

  if (attr) {
    auto def = attr->m_definition;
    auto &type = def->type;

    if (type.base().get_class() != types::type_class::E_COMPOSITE_FUNCTION) {
      report(def->loc());
      return;
    }

    auto &cast_type = static_cast<types::type_composite_function &>(def->type.base());
    if (check_func_parameter_list(cast_type, def->loc(), match_types)) ref.type = cast_type.return_type();
    return;
  }

  error_report error = {
      {fmt::format("Unknown function to call `{}`", name), ref.loc()}
  };

  report_error(error);
}

bool semantic_analyzer::analyze_main(ast::i_ast_node &ref) {
  // If we should visit the main scope, then we won't enter a function_definition node and set this flag ourselves.
  // This flag prevents the analyzer to go lower than 1 layer of functions;
  m_in_function_body = true;
  next_value_block();

  auto block_ptr = ezvis::visit<ast::statement_block *, ast::error_node, ast::statement_block>(
      paracl::utils::visitors{
          [&](const ast::error_node &e) {
            analyze_node(e);
            return nullptr;
          },
          [](ast::statement_block &s) { return &s; }},
      ref
  );

  if (block_ptr) {
    auto &main_block = *block_ptr;
    m_functions->global_stab = &main_block.stab;
    analyze_node(main_block);

    for (const auto &st : main_block.return_statements) {
      expect_type_eq(*st, types::type_builtin::type_void());
    }
  }

  return m_error_queue->empty();
}

bool semantic_analyzer::analyze_func(ast::function_definition &ref, bool is_recursive) {
  m_type_errors_allowed = is_recursive;

  analyze_node(ref);
  if (is_recursive) {
    analyze_func(ref, false);
  }

  return m_error_queue->empty();
}

void semantic_analyzer::analyze_node(ast::function_definition_to_ptr_conv &ref) {
  ref.type = types::generic_type::make<types::type_composite_function>(ref.definition().type);
}

void semantic_analyzer::analyze_node(ast::return_statement &ref) {
  m_return_statements->push_back(&ref);
  if (!ref.empty()) {
    apply(ref.expr());
    ref.type = ref.expr().type;
  } else {
    ref.type = types::type_builtin::type_void();
  }
}

} // namespace paracl::frontend