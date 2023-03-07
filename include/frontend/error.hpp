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

#include "location.hpp"

#include <string>
#include <vector>

namespace paracl::frontend {

struct error_kind final {
  std::string m_error_message;
  location m_loc;
};

struct error_attachment final {
  std::string m_info_message;
  location m_loc;
};

struct error_report final {
  error_kind m_primary_error;
  std::vector<error_attachment> m_attachments;

public:
  error_report(const error_kind &primary) : m_primary_error(primary) {}
  void add_attachment(error_attachment attach) { m_attachments.push_back(std::move(attach)); }
};

} // namespace paracl::frontend