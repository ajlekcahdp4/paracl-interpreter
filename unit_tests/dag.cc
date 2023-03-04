/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "dag/dag.hpp"

#include <gtest/gtest.h>

using dag = paracl::containers::dag<int>;

TEST(test_dag, test_insert_edge) {
  dag A;
  EXPECT_NO_THROW(A.insert_edge(1, 2));
  EXPECT_NO_THROW(A.insert_edge(1, 3));
  EXPECT_NO_THROW(A.insert_edge(1, 4));
  EXPECT_NO_THROW(A.insert_edge(2, 4));
  EXPECT_NO_THROW(A.insert_edge(4, 3));
  EXPECT_THROW(A.insert_edge(1, 2), std::logic_error); // already exist
  EXPECT_THROW(A.insert_edge(3, 2), std::logic_error); // creates cycle

  EXPECT_EQ(A.number_of_edges(), 5);
  EXPECT_EQ(A.number_of_vertices(), 4);
  EXPECT_TRUE(A.vertex_exists(1));
  EXPECT_TRUE(A.vertex_exists(2));
  EXPECT_TRUE(A.vertex_exists(3));
  EXPECT_TRUE(A.vertex_exists(4));
  EXPECT_FALSE(A.vertex_exists(11));
  EXPECT_TRUE(A.edge_exists(1, 2));
  EXPECT_TRUE(A.edge_exists(1, 3));
  EXPECT_TRUE(A.edge_exists(1, 4));
  EXPECT_FALSE(A.edge_exists(3, 2)); // cycle
}