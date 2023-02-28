/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <list>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace paracl::containers {

template <typename T> class dag {
public:
  using size_type = std::size_t;
  using value_type = T;

private:
  std::unordered_map<value_type, std::vector<value_type>> m_adj_list;
  size_type m_edge_n = 0;

public:
  dag() = default;

  virtual ~dag() {}

  void insert_vertex(const value_type &val) {
    auto &&[iter, inserted] = m_adj_list.insert({val, {}});
    if (!inserted) throw std::logic_error{"Attempt to insert existing vertex into a dag"};
  }

  void insert_edge(const value_type &vert1, const value_type &vert2) {
    if (!m_adj_list.find(vert2)) insert_vertex(vert2);
    auto &&list1 = m_adj_list[vert1];
    if (std::find(list1, vert2) != list1.end()) throw std::logic_error{"Attempt to insert existing edge into a dag"};
    list1.push_back(vert2);
    ++m_edge_n;
  }

  size_type number_of_edges() const { return m_edge_n; }

  size_type number_of_vertices() const { return m_adj_list.size(); }
};

} // namespace paracl::containers