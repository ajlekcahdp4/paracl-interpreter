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
#include <variant>

namespace paracl::frontend {

using types::type_builtin;

ast::statement_block *semantic_analyzer::try_get_statement_block_ptr(ast::i_ast_node &ref
) { // clang-format off
  return ezvis::visit<ast::statement_block *, ast::error_node, ast::statement_block>(
      ::utils::visitors{
          [this](const ast::error_node &e) { analyze_node(e); return nullptr; },
          [](ast::statement_block &s) { return &s; }},
      ref
  );
} // clang-format on

ast::value_block *semantic_analyzer::try_get_value_block_ptr(ast::i_ast_node &ref
) { // clang-format off
  return ezvis::visit<ast::value_block *, ast::error_node, ast::value_block>(
      ::utils::visitors{
          [this](const ast::error_node &e) { analyze_node(e); return nullptr; },
          [](ast::value_block &s) { return &s; }},
      ref
  );
} // clang-format on

void semantic_analyzer::analyze_node(ast::unary_expression &ref) {
  apply(ref.expr());
  if (expect_type_eq(ref.expr(), type_builtin::type_int)) {
    ref.type = type_builtin::type_int;
  }
}

void semantic_analyzer::analyze_node(ast::assignment_statement &ref) {
  apply(ref.right());

  auto &right_type = ref.right().type;
  if (!right_type) {
    if (!m_type_errors_allowed) {
      report_error(
          "Type of the right side of the assignment is unknown or can't be deduced",
          ref.right().loc()
      );
    }
    return;
  }

  if (right_type == types::type_builtin::type_void) {
    report_error("Type of the right side of the assignment can't be `void`", ref.right().loc());
    return;
  }

  for (auto &v : ref) {
    bool declared = false;
    ast::i_expression *expr = nullptr;
    if (std::holds_alternative<ast::variable_expression>(v)) {
      declared = analyze_node(std::get<ast::variable_expression>(v), true);
      expr = &std::get<ast::variable_expression>(v);
    }
    if (std::holds_alternative<ast::subscript>(v)) {
      analyze_node(std::get<ast::subscript>(v));
      expr = &std::get<ast::subscript>(v);
    }
    if (right_type && !declared && !expr->type) {
      expr->type = right_type;
    } else {
      if (expect_type_eq(*expr, ref.right().type)) continue;
      return;
    }
  }

  ref.type = ref.right().type;
}

void semantic_analyzer::analyze_node(ast::binary_expression &ref) {
  apply(ref.right());
  apply(ref.left());

  if (expect_type_eq(ref.right(), type_builtin::type_int) &&
      expect_type_eq(ref.left(), type_builtin::type_int)) {
    ref.type = type_builtin::type_int;
  }
}

void semantic_analyzer::analyze_node(ast::print_statement &ref) {
  apply(ref.expr());
  expect_type_eq(ref.expr(), type_builtin::type_int);
}

types::generic_type semantic_analyzer::deduce_return_type(location loc) {
  const auto on_error = [this](location loc) {
    error_report error = {
        {fmt::format("Return type deduction failed, found mismatch"), loc}
    };
    report_error(error);
  };

  if (m_return_statements->empty()) return types::type_builtin::type_void;

  auto types_not_equal = [](ast::return_statement *first, ast::return_statement *second) {
    return first->expr().type && second->expr().type && (first->expr().type != second->expr().type);
  };

  if (std::adjacent_find(
          m_return_statements->begin(), m_return_statements->end(), types_not_equal
      ) != m_return_statements->end()) {
    on_error(loc);
    return types::type_builtin::type_void;
  }

  auto it = std::find_if(
      m_return_statements->begin(), m_return_statements->end(),
      [](ast::return_statement *ret) { return ret->expr().type; }
  );
  if (it == m_return_statements->end()) {
    on_error(loc);
    return types::type_builtin::type_void;
  }
  auto &&found = *it;
  return found->expr().type;
}

using expressions_and_base =
    ::utils::tuple_add_types_t<ast::tuple_expression_nodes, ast::i_ast_node>;
void semantic_analyzer::analyze_node(ast::value_block &ref) {
  m_scopes.begin_scope(ref.stab);

  auto *old_returns = m_return_statements;
  m_return_statements = &ref.return_statements;
  ref.return_statements.clear();

  for (auto start = ref.begin(), finish = ref.end(); start != finish; ++start) {
    assert(*start && "Broken statement pointer in a block");
    auto &&stmt = **start;
    apply(stmt);

    bool is_last = (std::next(start) == finish);
    if (!is_last) continue;
    /* There we've already reached the last statement of the value block. It may be an implicit
     * return */

    /* If the last expression is of type void, then this's not an implicit return */
    auto type = ezvis::visit_tuple<types::generic_type, expressions_and_base>(
        ::utils::visitors{
            [](ast::i_expression &expr) { return expr.type; },
            [](ast::i_ast_node &) { return type_builtin::type_void; }
        },
        stmt
    );
    auto is_implicit_return = (type != types::type_builtin::type_void);

    /* It also can be an explicit return */
    auto is_return = ezvis::visit<bool, ast::return_statement, ast::i_ast_node>(
        ::utils::visitors{
            [](ast::return_statement &) { return true; }, [](ast::i_ast_node &) { return false; }
        },
        stmt
    );

    if (!is_implicit_return || is_return) break;

    /* There stmt is definitely an implicit return */
    auto expr_ptr = ezvis::visit_tuple<ast::i_expression *, ast::tuple_expression_nodes>(
        [](ast::i_expression &expr) { return &expr; }, stmt
    );
    auto &&ret = m_ast->make_node<ast::return_statement>(expr_ptr, expr_ptr->loc());
    m_return_statements->push_back(&ret);

    /* replace expression node in AST with explicit return statement */
    *start = &ret;
  }

  auto type = deduce_return_type(ref.loc());
  ref.type = type;

  m_return_statements = old_returns;
}

void semantic_analyzer::analyze_node(ast::statement_block &ref) {
  m_scopes.begin_scope(ref.stab);
  for (auto start = ref.begin(), finish = ref.end(); start != finish; ++start) {
    assert(*start && "Broken statement pointer in a block");
    auto &&stmt = **start;
    apply(stmt);

    bool is_last = (std::next(start) == finish);
    if (!is_last || !m_return_statements) continue;
    /* There we've already reached the last statement of the value block. It may be an implicit
     * return */

    /* If the last expression is of type void, then this's not an implicit return */
    auto type = ezvis::visit_tuple<types::generic_type, expressions_and_base>(
        ::utils::visitors{
            [](ast::i_expression &expr) { return expr.type; },
            [](ast::i_ast_node &) { return type_builtin::type_void; }
        },
        stmt
    );
    bool is_implicit_return = (!type || type != types::type_builtin::type_void);

    /* It also can be an explicit return */
    bool is_return = ezvis::visit<bool, ast::return_statement, ast::i_ast_node>(
        ::utils::visitors{
            [](ast::return_statement &) { return true; }, [](ast::i_ast_node &) { return false; }
        },
        stmt
    );

    if (!is_implicit_return || is_return) break;

    /* There stmt is definitely an implicit return */
    auto expr_ptr = ezvis::visit_tuple<ast::i_expression *, ast::tuple_expression_nodes>(
        [](ast::i_expression &expr) { return &expr; }, stmt
    );
    auto &&ret = m_ast->make_node<ast::return_statement>(expr_ptr, expr_ptr->loc());
    m_return_statements->push_back(&ret);

    /* replace expression node in AST with explicit return statement */
    *start = &ret;
  }
}

void semantic_analyzer::analyze_node(ast::if_statement &ref) {
  apply(*ref.cond());
  expect_type_eq(*ref.cond(), type_builtin::type_int.base());
  apply(*ref.true_block());
  if (ref.else_block()) apply(*ref.else_block());
}

void semantic_analyzer::analyze_node(ast::while_statement &ref) {
  apply(*ref.cond());
  expect_type_eq(*ref.cond(), type_builtin::type_int);
  apply(*ref.block());
}

void semantic_analyzer::analyze_node(ast::subscript &ref) {
  auto attr = m_scopes.lookup_symbol(ref.name());
  if (!attr) {
    report_error(fmt::format("Use of undeclared variable `{}`", ref.name()), ref.loc());
    return;
  }
  apply(*ref.get_subscript());
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
  ref.type = ezvis::visit<types::generic_type, ast::variable_expression>(
      [](ast::variable_expression &v) { return v.type; }, *attr->m_definition
  );

  return true;
}

void semantic_analyzer::analyze_node(ast::function_call &ref) {
  for (auto &&param : ref) {
    apply(*param);
  }

  auto name = ref.name();
  auto function_found = m_functions->named_functions.lookup(name);
  auto attr = m_scopes.lookup_symbol(name);

  const auto report = [this, name, &ref](auto &&loc) {
    if (m_type_errors_allowed) return;

    error_report error = {
        {fmt::format("Call parameter type/count mismatch", name), ref.loc()}
    };

    error.add_attachment({fmt::format("Defined here", name), loc});
    report_error(error);
  };

  const auto match_types = [this](auto expr_ptr, auto &&arg) {
    return expect_type_eq(*expr_ptr, arg.base());
  };
  const auto check_func_parameter_list = [ref, report](auto &&type, auto &&loc, auto match) {
    if (std::mismatch(ref.begin(), ref.end(), type.cbegin(), type.cend(), match).first !=
            ref.end() ||
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
      report.add_attachment(
          {fmt::format("[Info] Have variable `{}`", name), attr->m_definition->loc()}
      );
      report.add_attachment(
          {fmt::format("[Info] Have function `{}`", name), function_found->definition->loc()}
      );
      report_error(report);
      return;
    }

    if (check_func_parameter_list(
            function_found->definition->type, function_found->definition->loc(), match_types
        )) {
      ref.type = function_found->definition->type.return_type();
    }

    return;
  }

  if (attr) {
    auto *def = attr->m_definition;
    auto &type = def->type;

    if (type.base().get_class() != types::type_class::E_COMPOSITE_FUNCTION) {
      report(def->loc());
      return;
    }

    auto &cast_type = static_cast<types::type_composite_function &>(def->type.base());
    if (check_func_parameter_list(cast_type, def->loc(), match_types))
      ref.type = cast_type.return_type();
    return;
  }

  error_report error = {
      {fmt::format("Unknown function to call `{}`", name), ref.loc()}
  };

  report_error(error);
}

bool semantic_analyzer::analyze_main(ast::i_ast_node &ref) {
  auto *block_ptr = try_get_statement_block_ptr(ref);

  if (block_ptr) {
    auto &main_block = *block_ptr;
    m_functions->global_stab = &main_block.stab;
    analyze_node(main_block);

    for (const auto *st : main_block.return_statements) {
      assert(st);
      expect_type_eq(st->type(), types::type_builtin::type_void, st->loc());
    }
  }

  return m_error_queue->empty();
}

bool semantic_analyzer::analyze_func(ast::function_definition &ref, bool is_recursive) {
  m_type_errors_allowed = is_recursive;
  m_scopes.begin_scope(ref.param_stab);
  auto *body_ptr = try_get_value_block_ptr(ref.body());
  /* If not Error node */
  if (body_ptr) {
    auto &&body = *body_ptr;
    analyze_node(body);
    ref.type.m_return_type = body.type;
  }
  if (is_recursive) analyze_func(ref, false);
  return m_error_queue->empty();
}

void semantic_analyzer::analyze_node(ast::function_definition &ref) {
  semantic_analyzer analyzer{*m_functions
  }; // Yes, yes, a semantic analyzer that recursively creates another sema is
     // pretty bad, but such is the language we are compiling.
  analyzer.set_error_queue(*m_error_queue);
  analyzer.set_ast(*m_ast);
  analyzer.m_scopes = m_scopes;
  auto attr = m_functions->named_functions.lookup(ref.name.value());
  bool is_recursive = (attr ? attr->recursive : false);
  analyzer.analyze_func(ref, is_recursive);
}

void semantic_analyzer::analyze_node(ast::function_definition_to_ptr_conv &ref) {
  analyze_node(ref.definition());
  ref.type = types::generic_type::make<types::type_composite_function>(ref.definition().type);
}

void semantic_analyzer::analyze_node(ast::return_statement &ref) {
  m_return_statements->push_back(&ref);
  if (!ref.empty()) {
    apply(ref.expr());
  }
}

} // namespace paracl::frontend
