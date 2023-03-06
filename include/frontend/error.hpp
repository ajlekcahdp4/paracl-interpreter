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
  std::string error_message;
  location loc;
};

struct error_attachment final {
  std::string info_message;
  location loc;
};

struct error_report final {
  error_kind primary_error;
  std::vector<error_attachment> attachments;

public:
  void add_attachment(error_attachment attach) { attachments.push_back(std::move(attach)); }
};

} // namespace paracl::frontend