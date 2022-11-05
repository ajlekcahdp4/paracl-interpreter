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

#include "i_ast_node.hpp"
#include "location.hpp"

#include <string>

namespace paracl::frontend::ast {

class error_node : public i_ast_node {
private:
  std::string m_error_message;
  location    m_loc;

public:
  error_node(std::string p_error, location p_l) : m_error_message{p_error}, m_loc{p_l} {};

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }
};

static inline i_ast_node_uptr make_error_node(std::string error, location l) {
  return std::make_unique<error_node>(error, l);
}

} // namespace paracl::frontend::ast