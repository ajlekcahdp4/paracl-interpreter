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

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string_view>
#include <vector>

namespace paracl::frontend {

class source_input {
private:
  std::string m_filename, m_file_source;
  using line_vec = std::vector<std::string>;
  line_vec m_file_lines;

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

  std::string_view getline(unsigned i) const {
    assert(i != 0 && "Line number can't be equal to 1");
    return m_file_lines.at(i - 1); /* Bison lines start with 1, so we have to subtrack */
  }

  // Can't make this const qualified, because bison location requires it be a modifiable pointer for whatever reason.
  std::string *filename() { return &m_filename; }
  std::unique_ptr<std::istringstream> iss() const { return std::make_unique<std::istringstream>(m_file_source); }
};

} // namespace paracl::frontend