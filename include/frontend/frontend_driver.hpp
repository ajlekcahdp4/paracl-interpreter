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

#include "bison_paracl_parser.hpp"
#include "error.hpp"
#include "frontend/ast/ast_container.hpp"
#include "scanner.hpp"

#include <vector>

namespace paracl::frontend {

class frontend_driver final {
private:
  scanner m_scanner;
  parser  m_parser;

  std::optional<error_kind> m_current_error;
  ast::ast_container        m_ast;

  friend class parser;
  friend class scanner;

private:
  void report_error(std::string message, location l) { m_current_error = {message, l}; }

  error_kind take_error() {
    auto error = m_current_error.value();
    m_current_error = std::nullopt;
    return error;
  }

public:
  frontend_driver() : m_scanner{*this}, m_parser{m_scanner, *this} {}

  bool parse() { return m_parser.parse(); }
  void switch_input_stream(std::istream *is) { m_scanner.switch_streams(is, nullptr); }

  ast::ast_container take_ast() && { return std::move(m_ast); }
  ast::ast_container take_ast() & { return m_ast; }
};
} // namespace paracl::frontend