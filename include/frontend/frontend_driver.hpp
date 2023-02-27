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
#include "frontend/ast/ast_container.hpp"
#include "frontend/error.hpp"
#include "frontend/scanner.hpp"
#include "frontend/semantic_analyzer.hpp"
#include "frontend/types/types.hpp"
#include "scanner.hpp"
#include "semantic_analyzer.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace paracl::frontend {

struct builtin_types {
  types::shared_type m_void = std::make_shared<types::type_builtin>(types::builtin_type_class::e_builtin_void);
  types::shared_type m_int = std::make_shared<types::type_builtin>(types::builtin_type_class::e_builtin_int);
};

class parser_driver {
private:
  scanner m_scanner;
  parser m_parser;
  semantic_analyzer m_semantic_analyzer;

  std::optional<error_kind> m_current_error;
  ast::ast_container m_ast;

  builtin_types m_types;

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
  parser_driver(std::string *filename) : m_scanner{*this, filename}, m_parser{m_scanner, *this}, m_types{} {}

  bool parse() { return m_parser.parse(); }

  void switch_input_stream(std::istream *is) { m_scanner.switch_streams(is, nullptr); }

  template <typename t_node_type, typename... t_args> t_node_type *make_ast_node(t_args &&...args) {
    return &m_ast.make_node<t_node_type>(std::forward<t_args>(args)...);
  }

  types::shared_type void_type_ptr() & { return m_types.m_void; }

  types::shared_type int_type_ptr() & { return m_types.m_int; }

  void set_ast_root_ptr(ast::i_ast_node *ptr) { // nullptr is possible
    m_ast.set_root_ptr(ptr);
  }

  ast::ast_container &ast() & { return m_ast; }
  ast::i_ast_node *get_ast_root_ptr() & { return m_ast.get_root_ptr(); }
};

class source_input {
private:
  std::string m_filename;
  std::string m_file_source;
  std::vector<std::string> m_file_lines;

private:
  void fill_lines() {
    std::istringstream iss{m_file_source};
    for (std::string line; std::getline(iss, line);) {
      m_file_lines.push_back(line);
    }
  }

public:
  source_input(std::filesystem::path input_path) : m_filename{input_path} {
    std::ifstream ifs;
    ifs.exceptions(ifs.exceptions() | std::ios::failbit);
    ifs.open(input_path, std::ios::binary);

    std::stringstream ss;
    ss << ifs.rdbuf();
    m_file_source = ss.str();

    fill_lines();
  }

  std::string_view getline(unsigned i) {
    assert(i != 0 && "Line number can't be equal to 1");
    return m_file_lines.at(i - 1); /* Bison lines start with 1, so we have to subtrack */
  }

  std::string *filename() { return &m_filename; }
  std::unique_ptr<std::istringstream> iss() const { return std::make_unique<std::istringstream>(m_file_source); }
};

class frontend_driver {
private:
  source_input m_source;
  std::unique_ptr<std::istringstream> m_iss;

  std::unique_ptr<parser_driver> m_parsing_driver;
  semantic_analyzer m_semantic_analyzer;

private:
  void report_pretty_error(error_kind err) {
    auto [msg, loc] = err;

    const std::string bison_syntax = "syntax error";
    if (err.error_message.starts_with(bison_syntax)) {
      auto &str =
          err.error_message; // Hacky workaround to capitalize bison syntax error. Should rework later. TODO[Sergei]
      str.replace(str.find(bison_syntax), bison_syntax.length(), "Syntax error");
    }

    const auto make_squigly_line = [](int column) {
      constexpr int max_squigly_width = 4;
      int squigly_width = std::min(max_squigly_width, column - 1);

      std::stringstream ss;
      for (int i = 0; i < column - squigly_width - 1; ++i) {
        ss << " ";
      }

      const auto make_squigly = [&](auto width) {
        for (int i = 0; i < width; ++i) {
          ss << "~";
        }
      };

      make_squigly(squigly_width);
      ss << "^";
      make_squigly(max_squigly_width);

      return ss.str();
    };

    std::cout << loc << ": " << err.error_message << "\n";
    if (loc.begin.line == loc.end.line) {
      std::cout << loc.begin.line << "\t| " << m_source.getline(loc.begin.line) << "\n";
      std::cout << "\t  " << make_squigly_line(loc.begin.column) << "\n";
      return;
    }

    // Here we handle multiline errors
    std::cout << "\t" << loc.begin.line << " | " << m_source.getline(loc.begin.line) << "\n";
    for (auto start = loc.begin.line + 1, finish = loc.end.line; start <= finish; ++start) {
      std::cout << "\t" << m_source.getline(loc.begin.line) << "\n";
    }
  }

public:
  frontend_driver(std::filesystem::path input_path)
      : m_source{input_path}, m_iss{m_source.iss()},
        m_parsing_driver{std::make_unique<parser_driver>(m_source.filename())}, m_semantic_analyzer{} {
    m_parsing_driver->switch_input_stream(m_iss.get());
  }

  const ast::ast_container &ast() const & { return m_parsing_driver->ast(); }
  void parse() { m_parsing_driver->parse(); }

  bool analyze() {
    auto &ast = m_parsing_driver->ast();
    if (!ast.get_root_ptr()) return true;

    std::vector<paracl::frontend::error_kind> errors;
    bool valid = m_semantic_analyzer.analyze(*ast.get_root_ptr(), errors);

    for (const auto &e : errors) {
      report_pretty_error(e);
    }

    return valid;
  }
};

  } // namespace paracl::frontend