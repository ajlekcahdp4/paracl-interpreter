/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/call_collector.hpp"

namespace paracl::frontend {

void call_collector::collect_calls(ast::assignment_statement &ref) {
  apply(ref.right());
}

void call_collector::collect_calls(ast::binary_expression &ref) {
  apply(ref.right());
  apply(ref.left());
}

void call_collector::collect_calls(ast::if_statement &ref) {
  apply(ref.cond());
  apply(ref.true_block());
  if (ref.else_block() != nullptr) apply(*ref.else_block());
}

void call_collector::collect_calls(ast::print_statement &ref) {
  apply(ref.expr());
}

void call_collector::collect_calls(ast::statement_block &ref) {
  for (auto &statement : ref) {
    assert(statement);
    apply(*statement);
  }
}

void call_collector::collect_calls(ast::unary_expression &ref) {
  apply(ref.expr());
}

void call_collector::collect_calls(ast::while_statement &ref) {
  apply(ref.cond());
  apply(ref.block());
}

void call_collector::collect_calls(ast::return_statement &ref) {
  apply(ref.expr());
}

void call_collector::collect_calls(ast::function_call &ref) {
  m_calls->push_back(&ref);
}

} // namespace paracl::frontend