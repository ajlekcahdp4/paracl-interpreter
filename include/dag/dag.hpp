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

#include "directed_graph.hpp"

namespace paracl::containers {

template <typename T> struct dag : public directed_graph<graph_node<T>> {
  using base = directed_graph<graph_node<T>>;
  using base::insert_edge;
  using base::insert_vertex;
  using base::vertex_exists;
  using typename base::value_type;
  void insert_edge(const value_type &first, const value_type &second) override {
    if (!vertex_exists(first)) insert_vertex(first);
    if (!vertex_exists(second)) insert_vertex(second);
    breadth_first_search search{*this};
    auto &&cycle_possible = search(second, first);
    if (cycle_possible) throw std::logic_error{"Attempt to create cycle in DAG"};
    base::insert_edge_base(first, second);
  }
};

} // namespace paracl::containers