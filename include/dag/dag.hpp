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

#include <algorithm>
#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace paracl::containers {

namespace detail {

template <typename T> class dag_node_base {
  T m_val;

public:
  using value_type = T;
  virtual ~dag_node_base() {}

  dag_node_base(const value_type &val) : m_val{val} {}

  operator value_type() { return m_val; }
};

template <typename T> class colored_dag_node : public dag_node_base<T> {
public:
  using typename dag_node_base<T>::value_type;

  enum class node_color {
    E_WHITE,
    E_GRAY
  };

private:
  node_color m_color = node_color::E_WHITE;

public:
  colored_dag_node(const value_type &val, const node_color color = node_color::E_WHITE)
      : dag_node_base<value_type>{val}, m_color{color} {}
};

template <typename node_t> class dag_base {
public:
  using size_type = std::size_t;
  using value_type = typename node_t::value_type;

private:
  std::unordered_map<value_type, std::vector<node_t>> m_adj_list;
  size_type m_edge_n = 0;

public:
  dag_base(){};

  virtual ~dag_base() {}

  void insert_vertex(const value_type &val) {
    auto &&[iter, inserted] = m_adj_list.insert({val, {}});
    if (!inserted) throw std::logic_error{"Attempt to insert existing vertex into a dag"};
  }

  // inserts vertices if they are not already at the DAG
  void insert_edge(const value_type &vert1, const value_type &vert2) {
    if (!m_adj_list.contains(vert2)) insert_vertex(vert2);
    auto &&list1 = m_adj_list[vert1];
    if (std::find(list1.begin(), list1.end(), vert2) != list1.end())
      throw std::logic_error{"Attempt to insert existing edge into a dag"};
    list1.push_back(node_t{vert2});
    ++m_edge_n;
  }

  size_type number_of_edges() const { return m_edge_n; }

  size_type number_of_vertices() const { return m_adj_list.size(); }

  bool empty() const { return number_of_vertices(); }

  bool vertex_exists(const value_type &val) const { return m_adj_list.contains(val); }

  bool edge_exists(const value_type &first, const value_type &second) const {
    if (!m_adj_list.constains(first) || !m_adj_list.contains(second)) return false;
    auto &&list = m_adj_list.at(first);
    if (std::find(list.begin(), list.end(), second) == list.end()) return false;
    return true;
  }

  size_type number_of_successors(const value_type &val) const {
    if (!m_adj_list.contains(val)) throw std::logic_error{"Attempt to get number of successors of non-existent vertex"};
    return m_adj_list[val].size();
  }

  auto begin() { return m_adj_list.begin(); }
  auto end() { return m_adj_list.end(); }
  auto begin() const { return m_adj_list.cbegin(); }
  auto end() const { return m_adj_list.cend(); }
  auto cbegin() { return m_adj_list.cbegin(); }
  auto cend() { return m_adj_list.cend(); }
};

} // namespace detail

template <typename T> using colored_dag = typename detail::dag_base<detail::colored_dag_node<T>>;

} // namespace paracl::containers