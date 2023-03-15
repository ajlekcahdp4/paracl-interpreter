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
  set_state(semantic_analysis_state::E_RVALUE);
  apply(ref.right());
  set_state(semantic_analysis_state::E_LVALUE);

  auto &right_type = ref.right().type;
  if (!right_type) {
    report_error("Type of the right side of the assignment is unknown or can't be deduced", ref.right().loc());
    reset_state();
    return;
  }

  for (auto &v : ref) {
    bool declared = analyze_node(v);
    if (right_type && !declared && !v.type) {
      v.type = right_type;
    } else {
      if (expect_type_eq(v, ref.right().type)) continue;
      reset_state();
      return;
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

  if (expect_type_eq(ref.right(), type_builtin::type_int()) && expect_type_eq(ref.left(), type_builtin::type_int())) {
    ref.type = type_builtin::type_int();
  }

  reset_state();
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  set_state(semantic_analysis_state::E_RVALUE);

  apply(ref.expr());
  expect_type_eq(ref.expr(), type_builtin::type_int());

  reset_state();
}

void semantic_analyzer::check_return_types_matches(ast::function_definition &ref) {
  bool valid = true;

  const auto on_error = [&](location loc) {
    error_report error = {
        {fmt::format("Return type deduction failed, found mismatch"), loc}
    };

    report_error(error);
    valid = false;
  };

  auto &ret_type = ref.type.m_return_type;
  if (m_return_statements.empty()) {
    if (!ret_type || ret_type == types::type_builtin::type_void()) {
      ret_type = types::type_builtin::type_void();
      return;
    }

    on_error(ref.loc());
    return;
  }

  auto first_type = m_return_statements.front()->type;

  if (ret_type) {
    for (const auto &st : m_return_statements) {
      assert(st && "[Dedug]: Broken statement pointer");
      if (st->type == ret_type) continue;
      on_error(st->loc());
    }
  }

  else {
    for (const auto &st : m_return_statements) {
      assert(st && "[Dedug]: Broken statement pointer");
      if (st->type == first_type) continue;
      on_error(st->loc());
    }
  }

  if (valid) ref.type.m_return_type = first_type;
}

using expressions_and_base = utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node>;
void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  begin_scope(ref.stab);
  ref.type = types::type_builtin::type_void();

  for (auto start = ref.cbegin(), finish = ref.cend(); start != finish; ++start) {
    auto ptr = *start;
    assert(ptr && "[Debug]: broken statement pointer in a block");
    auto &st = *ptr;
    apply(st);

    bool is_last = (start == std::prev(finish));
    if (!is_last) continue;

    auto type = ezvis::visit_tuple<types::generic_type, expressions_and_base>(
        paracl::utils::visitors{
            [](ast::i_expression &expr) { return expr.type; },
            [&](ast::i_ast_node &) { return type_builtin::type_void(); }},
        st
    );

    ref.type = type;
  }

  end_scope();
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  begin_scope(*ref.control_block_symtab());
  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int().base());

  begin_scope(*ref.true_symtab());
  apply(ref.true_block());
  end_scope();

  if (ref.else_block() != nullptr) {
    begin_scope(*ref.else_symtab());
    apply(*ref.else_block());
    end_scope();
  }

  end_scope();
}

void semantic_analyzer::analyze_node(ast::while_statement &ref) {
  begin_scope(*ref.symbol_table());

  apply(ref.cond());
  expect_type_eq(ref.cond(), type_builtin::type_int());
  apply(ref.block());

  end_scope();
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

using expressions_and_return =
    utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node, ast::return_statement>;

void semantic_analyzer::analyze_node(ast::function_definition &ref) {
  if (m_in_function_body) return;
  m_in_function_body = true; // Set flag

  begin_scope(ref.param_stab);

  auto &&body = ref.body();
  auto body_type = ast::identify_node(body);
  assert(
      body_type == ast::ast_node_type::E_ERROR_NODE ||
      body_type == ast::ast_node_type::E_STATEMENT_BLOCK && "Broken statement block pointer"
  );

  if (body_type == ast::ast_node_type::E_ERROR_NODE) {
    apply(body);                // Visit error node
    m_in_function_body = false; // Exit
    end_scope();
    return;
  }

  // The only other possibility for the reference is statement block.
  auto &st_block = static_cast<ast::statement_block &>(body);

  apply(st_block);
  check_return_types_matches(ref);
  end_scope();

  m_in_function_body = false; // Exit
}

void semantic_analyzer::analyze_node(ast::function_call &ref) {
  for (auto &&param : ref) {
    apply(*param);
  }

  auto name = ref.name();
  auto function_found = m_functions->m_named.lookup(std::string{name});
  auto attr = m_scopes.lookup_symbol(name);

  const auto report = [&](auto &&loc) {
    error_report error = {
        {fmt::format("Call parameter type/count mismatch", name), ref.loc()}
    };

    error.add_attachment({fmt::format("Defined here", name), loc});
    report_error(error);
  };

  const auto match_expr_type = [&](auto *expr_ptr, auto &&arg) { return expect_type_eq(*expr_ptr, arg.type.base()); };
  const auto match_types = [&](auto *expr_ptr, auto &&arg) { return expect_type_eq(*expr_ptr, arg.base()); };

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
    }

    if (check_func_parameter_list(*function_found, function_found->loc(), match_expr_type)) {
      ref.type = function_found->type.return_type();
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

void semantic_analyzer::analyze_node(ast::function_definition_to_ptr_conv &ref) {
  ref.type = types::generic_type::make<types::type_composite_function>(ref.definition().type);
}

void semantic_analyzer::analyze_node(ast::return_statement &ref) {
  m_return_statements.push_back(&ref);
  if (!ref.empty()) apply(ref.expr());
  ref.type = ref.expr().type;
}

} // namespace paracl::frontend