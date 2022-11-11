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

public:
  error_node(std::string msg, location l) : i_ast_node{l}, m_error_message{msg} {};

  error_node(const error_node &) = delete;
  error_node &operator=(const error_node &) = delete;

  void accept(i_ast_visitor &visitor) override { visitor.visit(this); }

  i_ast_node_uptr clone() override;

  std::string error_msg() { return m_error_message; }
};

static inline i_ast_node_uptr make_error_node(std::string msg, location l) {
  return std::make_unique<error_node>(msg, l);
}

} // namespace paracl::frontend::ast