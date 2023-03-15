/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/dumper.hpp"
#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_nodes.hpp"
#include "utils/serialization.hpp"
#include <fmt/core.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend::ast {

class ast_dumper : public ezvis::visitor_base<const i_ast_node, ast_dumper, void> {
private:
  using to_visit = tuple_all_nodes;

public:
  std::vector<const i_ast_node *> m_queue;
  using oput_iter = std::back_insert_iterator<std::string>;
  oput_iter *m_iter = nullptr;

private:
  void print_declare_node(const i_ast_node &ref, std::string_view label) {
    assert(m_iter);
    fmt::format_to(*m_iter, "\tnode_{:x} [label = \"{}\"];\n", utils::pointer_to_uintptr(&ref), label);
  }

  void print_bind_node(const i_ast_node &parent, const i_ast_node &child, std::string_view label = "") {
    assert(m_iter);
    fmt::format_to(
        *m_iter, "\tnode_{:x} -> node_{:x} [label = \"{}\"]\n", utils::pointer_to_uintptr(&parent),
        utils::pointer_to_uintptr(&child), label
    );
  }

public:
  ast_dumper() = default;

  EZVIS_VISIT_CT(to_visit);

  void dump_node(const assignment_statement &ref);
  void dump_node(const binary_expression &ref);
  void dump_node(const if_statement &ref);
  void dump_node(const print_statement &ref);
  void dump_node(const statement_block &ref);
  void dump_node(const unary_expression &ref);
  void dump_node(const while_statement &ref);

  void dump_node(const read_expression &ref) { print_declare_node(ref, "<read> ?"); }
  void dump_node(const error_node &ref) { print_declare_node(ref, "<error>"); }

  void dump_node(const variable_expression &ref) {
    print_declare_node(ref, fmt::format("<identifier> {}\n<type> {}", ref.name(), ref.type_str()));
  }

  void dump_node(const constant_expression &ref) {
    print_declare_node(ref, fmt::format("<integer constant> {:d}", ref.value()));
  }

  void dump_node(const function_definition &ref);
  void dump_node(const return_statement &ref);
  void dump_node(const function_call &ref);
  void dump_node(const function_definition_to_ptr_conv &ref);

  EZVIS_VISIT_INVOKER(dump_node);

private:
  void add_next(const i_ast_node &node) { m_queue.push_back(&node); }

  const i_ast_node *take_next() {
    if (m_queue.empty()) return nullptr;
    auto ptr = m_queue.back();
    m_queue.pop_back();
    return ptr;
  }

public:
  std::string ast_dump(const i_ast_node &root) {
    std::string output;
    auto iterator = std::back_inserter(output);

    m_iter = &iterator;
    m_queue.clear();
    add_next(root);

    fmt::format_to(iterator, "digraph abstract_syntax_tree {{\n");
    while (auto ptr = take_next()) {
      apply(*ptr);
    }
    fmt::format_to(iterator, "}}\n");

    return output;
  }
};

void ast_dumper::dump_node(const assignment_statement &ref) {
  print_declare_node(ref, "<assignment>");
  const i_ast_node *prev = &ref;

  for (auto start = ref.begin(), finish = ref.end(); start != finish; ++start) {
    const auto curr_ptr = &(*start);

    add_next(*curr_ptr);
    print_bind_node(*prev, *curr_ptr);

    prev = curr_ptr;
  }

  print_bind_node(ref, ref.right());
  add_next(ref.right());
}

void ast_dumper::dump_node(const binary_expression &ref) {
  print_declare_node(ref, fmt::format("<binary expression>: {}", ast::binary_operation_to_string(ref.op_type())));

  print_bind_node(ref, ref.left());
  print_bind_node(ref, ref.right());

  add_next(ref.left());
  add_next(ref.right());
}

void ast_dumper::dump_node(const if_statement &ref) {
  print_declare_node(ref, "<if>");

  print_bind_node(ref, ref.cond(), "<condition>");
  print_bind_node(ref, ref.true_block(), "<then>");

  add_next(ref.cond());
  add_next(ref.true_block());

  if (ref.else_block()) {
    print_bind_node(ref, *ref.else_block(), "<else>");
    add_next(*ref.else_block());
  }
}

void ast_dumper::dump_node(const print_statement &ref) {
  print_declare_node(ref, "<print_statement>");
  print_bind_node(ref, ref.expr());
  add_next(ref.expr());
}

void ast_dumper::dump_node(const statement_block &ref) {
  print_declare_node(ref, fmt::format("<statement_block>\n<type>: {}", ref.type_str()));
  for (const auto &v : ref) {
    print_bind_node(ref, *v);
    add_next(*v);
  }
}

void ast_dumper::dump_node(const unary_expression &ref) {
  print_declare_node(ref, fmt::format("<binary expression> {}", ast::unary_operation_to_string(ref.op_type())));
  print_bind_node(ref, ref.expr());
  add_next(ref.expr());
}

void ast_dumper::dump_node(const while_statement &ref) {
  print_declare_node(ref, "<while>");

  print_bind_node(ref, ref.cond(), "<condition>");
  print_bind_node(ref, ref.block(), "<body>");

  add_next(ref.cond());
  add_next(ref.block());
}

void ast_dumper::dump_node(const function_definition &ref) {
  std::string label;
  auto iter = std::back_inserter(label);

  fmt::format_to(
      iter, "<function definition>: {}\n<arg count>: {}\n<type>: {}", ref.name().value_or("anonymous"), ref.size(),
      ref.m_type.to_string()
  );

  print_declare_node(ref, label);
  for (unsigned i = 0; const auto &v : ref) {
    print_bind_node(ref, v, fmt::format("arg {}", i++));
    add_next(v);
  }

  print_bind_node(ref, ref.body());
  add_next(ref.body());
}

void ast_dumper::dump_node(const return_statement &ref) {
  print_declare_node(ref, fmt::format("<return statement>\n<type>: {}", ref.type_str()));
  print_bind_node(ref, ref.expr(), "<expression>");
  add_next(ref.expr());
}

void ast_dumper::dump_node(const function_call &ref) {
  print_declare_node(ref, fmt::format("<function call>: {}\n<param count>: {}", ref.name(), ref.size()));

  for (unsigned i = 0; const auto &v : ref) {
    print_bind_node(ref, *v, fmt::format("param {}", i++));
    add_next(*v);
  }
}

void ast_dumper::dump_node(const function_definition_to_ptr_conv &ref) {
  print_declare_node(ref, "<function def to ptr implicit conversion>");
  print_bind_node(ref, ref.definition());
  add_next(ref.definition());
}

std::string ast_dump_str(const i_ast_node &node) {
  ast_dumper dumper;
  return dumper.ast_dump(node);
}

} // namespace paracl::frontend::ast