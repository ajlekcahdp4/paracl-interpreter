/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "dag/directed_graph.hpp"

#include <gtest/gtest.h>

using directed_graph = paracl::containers::basic_directed_graph<int>;

TEST(test_directed_graph, insert_vertex) {
  directed_graph A;
  EXPECT_EQ(A.number_of_vertices(), 0);
  EXPECT_NO_THROW(A.insert_vertex(1));
  EXPECT_NO_THROW(A.insert_vertex(2));
  EXPECT_THROW(A.insert_vertex(1), std::logic_error);
  EXPECT_EQ(A.number_of_vertices(), 2);
  EXPECT_TRUE(A.vertex_exists(1));
  EXPECT_TRUE(A.vertex_exists(2));
  EXPECT_FALSE(A.vertex_exists(4));
}

TEST(test_directed_graph, insert_edge) {
  directed_graph A;
  EXPECT_NO_THROW(A.insert_edge(1, 2));
  EXPECT_NO_THROW(A.insert_edge(1, 3));
  EXPECT_NO_THROW(A.insert_edge(3, 2));
  EXPECT_NO_THROW(A.insert_edge(3, 3));
  EXPECT_THROW(A.insert_edge(1, 2), std::logic_error);
  EXPECT_EQ(A.number_of_edges(), 4);
  EXPECT_EQ(A.number_of_vertices(), 3);
  EXPECT_TRUE(A.vertex_exists(1));
  EXPECT_TRUE(A.vertex_exists(2));
  EXPECT_TRUE(A.vertex_exists(3));
  EXPECT_TRUE(A.edge_exists(1, 2));
  EXPECT_TRUE(A.edge_exists(1, 3));
  EXPECT_TRUE(A.edge_exists(3, 2));
  EXPECT_TRUE(A.edge_exists(3, 3));
}

TEST(test_directed_graph, test_is_connected) {
  directed_graph A;
  A.insert_edge(3, 6);
  A.insert_edge(3, 5);
  A.insert_edge(5, 4);
  A.insert_edge(4, 2);
  A.insert_edge(2, 5);
  A.insert_edge(1, 2);
  A.insert_edge(1, 4);
  A.insert_edge(1, 1);

  EXPECT_TRUE(A.is_connected(3, 5));
  EXPECT_TRUE(A.is_connected(1, 2));
  EXPECT_TRUE(A.is_connected(1, 1));
  EXPECT_FALSE(A.is_connected(1, 6));
  EXPECT_FALSE(A.is_connected(1, 3));
  EXPECT_FALSE(A.is_connected(5, 3));
}

TEST(test_directed_graph, test_is_reachable) {
  directed_graph A;
  A.insert_edge(3, 6);
  A.insert_edge(3, 5);
  A.insert_edge(5, 4);
  A.insert_edge(4, 2);
  A.insert_edge(2, 5);
  A.insert_edge(1, 2);
  A.insert_edge(1, 4);
  A.insert_edge(1, 1);

  EXPECT_TRUE(A.is_reachable(3, 6));
  EXPECT_TRUE(A.is_reachable(3, 2));
  EXPECT_FALSE(A.is_reachable(3, 1));
}

TEST(test_directed_graph, test_BFS) {
  directed_graph A;
  A.insert_edge(3, 6);
  A.insert_edge(3, 5);
  A.insert_edge(5, 4);
  A.insert_edge(4, 2);
  A.insert_edge(2, 5);
  A.insert_edge(1, 2);
  A.insert_edge(1, 4);
  auto &&scheduled = paracl::containers::breadth_first_schedule(A, 3);
  std::vector<int> res{3, 6, 5, 4, 2};

  EXPECT_EQ(res, scheduled);
}