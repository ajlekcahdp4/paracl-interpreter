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

#include <cstdint>
#include <vector>

namespace paracl::bytecode_vm {

class constant_pool {
public:
  using size_type = typename std::vector<int>::size_type;

private:
  std::vector<int> m_constants;

public:
  size_type size() const { return m_constants.size(); }
  int at(size_type index) const { return m_constants.at(index); }
  void push_back(int constant) { m_constants.push_back(constant); }

  auto begin() { return m_constants.begin(); }
  auto end() { return m_constants.end(); }
  auto begin() const { return m_constants.cbegin(); }
  auto end() const { return m_constants.cend(); }
  auto cbegin() const { return m_constants.cbegin(); }
  auto cend() const { return m_constants.cend(); }
};

} // namespace paracl::bytecode_vm