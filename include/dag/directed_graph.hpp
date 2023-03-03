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
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace paracl::containers {

template <typename T> class i_graph_node : private std::vector<T> {
  T m_val;

  using vector = std::vector<T>;

public:
  using value_type = T;
  virtual ~i_graph_node() {}

  i_graph_node(const value_type val = value_type{}) : m_val{std::move(val)} {}

  operator value_type() { return m_val; }

  value_type value() const { return m_val; }

  void add_adj(const value_type &val) {
    if (std::find(begin(), end(), val) != end())
      throw std::logic_error{"Attempt to add existing edge into a oriented graph"};
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
  requires std::derived_from<node_t, i_graph_node<typename node_t::value_type>>
class i_directed_graph {
public:
  using size_type = std::size_t;
  using value_type = typename node_t::value_type;
  using node_type = node_t;

private:
  std::unordered_map<value_type, node_t> m_adj_list;
  size_type m_edge_n = 0;

public:
  i_directed_graph(){};

  virtual ~i_directed_graph() {}

  virtual void insert_vertex(const value_type &val) { insert_vertex_base(val); }

  virtual void insert_edge(const value_type &first, const value_type &second) { insert_edge_base(first, second); }

  bool vertex_exists(const value_type &val) const { return m_adj_list.contains(val); }

  bool edge_exists(const value_type &first, const value_type &second) const {
    if (!(m_adj_list.contains(first) && m_adj_list.contains(second))) return false;
    auto &&list = m_adj_list.at(first);
    if (std::find(list.begin(), list.end(), second) == list.end()) return false;
    return true;
  }

  size_type number_of_edges() const { return m_edge_n; }

  size_type number_of_vertices() const { return m_adj_list.size(); }

  bool empty() const { return !number_of_vertices(); }

  size_type number_of_successors(const value_type &val) const {
    if (!m_adj_list.contains(val)) throw std::logic_error{"Attempt to get number of successors of non-existent vertex"};
    return m_adj_list[val].size();
  }

  // returns true if first is directly connected to second
  bool is_connected(const value_type &first, const value_type &second) {
    if (!(m_adj_list.contains(first) && m_adj_list.contains(second)))
      throw std::logic_error{"Attempt to check for connection with non-existent vertex"};
    auto &&found = std::find(m_adj_list[first].begin(), m_adj_list[first].end(), second);
    if (found == m_adj_list[first].end()) return false;
    return true;
  }

  // returns true if second is reachable from the first
  bool is_reachable(const value_type &first, const value_type &second) {
    auto &&vec = breadth_first_schedule(*this, first);
    if (std::find(vec.begin(), vec.end(), second) == vec.end()) return false;
    return true;
  }

  auto find(const value_type &val) { return m_adj_list.find(val); }

  auto begin() { return m_adj_list.begin(); }
  auto end() { return m_adj_list.end(); }
  auto begin() const { return m_adj_list.cbegin(); }
  auto end() const { return m_adj_list.cend(); }
  auto cbegin() { return m_adj_list.cbegin(); }
  auto cend() { return m_adj_list.cend(); }

protected:
  void insert_vertex_base(const value_type &val) {
    auto &&[iter, inserted] = m_adj_list.insert({val, {val}});
    if (!inserted) throw std::logic_error{"Attempt to insert existing vertex into a graph"};
  }

  // inserts vertices if they are not already at the DG
  void insert_edge_base(const value_type &vert1, const value_type &vert2) {
    if (!m_adj_list.contains(vert2)) insert_vertex(vert2);
    auto &&node1 = (*(m_adj_list.insert({vert1, vert1}).first)).second;
    node1.add_adj(vert2);
    ++m_edge_n;
  }
};

template <typename T> using basic_directed_graph = i_directed_graph<i_graph_node<T>>;

template <typename graph_t>
  requires std::derived_from<graph_t, i_directed_graph<typename graph_t::node_type>>
std::vector<typename graph_t::value_type>
breadth_first_schedule(graph_t &graph, const typename graph_t::value_type &root_val) {

  using value_type = typename graph_t::value_type;
  enum class color_t {
    E_WHITE,
    E_GRAY,
    E_BLACK
  };

  struct bfs_node : public graph_t::node_type {
    int m_dist = -1;
    color_t m_color = color_t::E_WHITE;
    bfs_node *m_prev = nullptr;

    bfs_node(const value_type &val) : graph_t::node_type{val} {}
  };

  std::vector<value_type> scheduled;
  std::unordered_map<value_type, bfs_node> nodes;

  auto &&root = graph.find(root_val);
  if (root == graph.end()) throw std::logic_error{"Non-existing vertex root in BFS"};

  scheduled.push_back(root_val);
  bfs_node node{root_val};
  node.m_color = color_t::E_GRAY;
  node.m_dist = 0;
  node.m_prev = nullptr;
  nodes.insert({root_val, node});

  std::deque<value_type> que;
  que.push_back(root_val);
  while (!que.empty()) {
    auto &&curr = que.front(); // curr : T
    que.pop_front();
    auto &&curr_node = (*(nodes.insert({curr, curr}).first)).second;
    auto &&curr_graph_node = (*graph.find(curr)).second;
    for (auto &&adj : curr_graph_node) { // adj : T
      auto &&adj_node = (*(nodes.insert({adj, adj}).first)).second;
      if (adj_node.m_color == color_t::E_WHITE) {
        scheduled.push_back(adj);
        adj_node.m_color = color_t::E_GRAY;
        adj_node.m_dist = curr_node.m_dist + 1;
        adj_node.m_prev = &curr_node;
        que.push_back(adj);
      }
    }
    curr_node.m_color = color_t::E_BLACK;
  }
  return scheduled;
}

template <typename graph_t>
  requires std::derived_from<graph_t, i_directed_graph<typename graph_t::node_type>>
std::vector<typename graph_t::value_type>
recursive_topo_sort(graph_t &graph, const typename graph_t::value_type &root_val) {
  using value_type = typename graph_t::value_type;
  enum class color_t {
    E_WHITE,
    E_GRAY,
    E_BLACK
  };

  struct bfs_node : public graph_t::node_type {
    int m_start = -1, m_finish = -1;
    color_t m_color = color_t::E_WHITE;
    bfs_node *m_prev = nullptr;

    bfs_node(const value_type &val) : graph_t::node_type{val} {}
  };

  int time = 0;
  std::vector<value_type> scheduled;
  std::unordered_map<value_type, bfs_node> nodes;

  for (auto &&val : graph)
    nodes.insert({val.first, val.first});

  std::function<void(value_type)> dfs_visit = [&time, &nodes, &dfs_visit, &graph, &scheduled](const value_type &val) {
    ++time;
    auto &&cur_node = nodes.at(val);
    cur_node.m_start = time;
    cur_node.m_color = color_t::E_GRAY;
    auto &&graph_node = graph.find(val)->second;
    for (auto &&adj : graph_node) {
      auto &&adj_node = nodes.at(adj);
      if (adj_node.m_color == color_t::E_WHITE) {
        adj_node.m_prev = &cur_node;
        dfs_visit(adj);
      }
    }
    cur_node.m_color = color_t::E_BLACK;
    scheduled.push_back(val);
    ++time;
    cur_node.m_finish = time;
  };

  for (auto &&val : graph) {
    if (nodes.at(val.first).m_color == color_t::E_WHITE) dfs_visit(val.first);
  }
  return std::vector(scheduled.rbegin(), scheduled.rend());
}

} // namespace paracl::containers