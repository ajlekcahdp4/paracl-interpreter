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
  error_node(std::string msg, location l) : i_ast_node{l}, m_error_message{msg} {};

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }
};

static inline i_ast_node_uptr make_error_node(std::string msg, location l) {
  return std::make_unique<error_node>(msg, l);
}

} // namespace paracl::frontend::ast