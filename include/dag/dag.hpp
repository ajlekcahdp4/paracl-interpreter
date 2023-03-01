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
#include <concepts>
#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace paracl::containers {

template <typename T> class i_dag_node : private std::vector<T> {
  T m_val;

  using vector = std::vector<T>;

public:
  using value_type = T;
  virtual ~i_dag_node() {}

  i_dag_node(const value_type val = value_type{}) : m_val{std::move(val)} {}

  operator value_type() { return m_val; }

  void add_adj(const value_type &val) {
    if (std::find(begin(), end(), val) != end()) throw std::logic_error{"Attempt to add existing edge into a DAG"};
    vector::push_back(val);
  }

  using vector::begin;
  using vector::end;

  using vector::cbegin;
  using vector::cend;

  using vector::empty;
  using vector::size;
};

template <typename node_t>
  requires std::derived_from<node_t, i_dag_node<typename node_t::value_type>>
class i_dag {
public:
  using size_type = std::size_t;
  using value_type = typename node_t::value_type;

private:
  std::unordered_map<value_type, node_t> m_adj_list;
  size_type m_edge_n = 0;

public:
  i_dag(){};

  virtual ~i_dag() {}

  virtual void insert_vertex(const value_type &val) {
    auto &&[iter, inserted] = m_adj_list.insert({val, {val}});
    if (!inserted) throw std::logic_error{"Attempt to insert existing vertex into a dag"};
  }

  // inserts vertices if they are not already at the DAG
  virtual void insert_edge(const value_type &vert1, const value_type &vert2) {
    if (!m_adj_list.contains(vert2)) insert_vertex(vert2);
    auto &&node1 = m_adj_list[vert1];
    node1.add_adj(vert2);
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

  auto find(const value_type &val) { return m_adj_list.find(val); }

  auto begin() { return m_adj_list.begin(); }
  auto end() { return m_adj_list.end(); }
  auto begin() const { return m_adj_list.cbegin(); }
  auto end() const { return m_adj_list.cend(); }
  auto cbegin() { return m_adj_list.cbegin(); }
  auto cend() { return m_adj_list.cend(); }
};

template <typename T> using basic_dag = i_dag<i_dag_node<T>>;

template <typename T> std::vector<T> breadth_first_schedule(i_dag<T> &dag, const T &val) {
  std::vector<T> scheduled;
  auto &&root = dag.find(val);
  if (root == dag.end()) throw std::logic_error{"Non-existing vertex root in BFS"};
}

} // namespace paracl::containers