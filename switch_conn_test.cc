#include <gtest/gtest.h>
#include <vector>
#include "circuit.h"

TEST(SwitchBlockTest, conn1) {
  switch_block* sb = new switch_block(4);
  
  sb->connect(NORTH, SOUTH, 0);
  ASSERT_TRUE(sb->is_connected(NORTH,SOUTH,0));
  ASSERT_FALSE(sb->is_connected(NORTH,EAST,0));
  ASSERT_FALSE(sb->is_connected(NORTH,WEST,0));
  ASSERT_FALSE(sb->is_connected(SOUTH,WEST,0));
  ASSERT_FALSE(sb->is_connected(SOUTH,EAST,0));
  ASSERT_FALSE(sb->is_connected(WEST,EAST,0));
}

TEST(SwitchBlockTest, conn2) {
  switch_block* sb = new switch_block(4);
  
  sb->connect(NORTH, SOUTH, 0);
  sb->connect(SOUTH,EAST,0);
  ASSERT_TRUE(sb->is_connected(NORTH,SOUTH,0));
  ASSERT_TRUE(sb->is_connected(NORTH,EAST,0));
  ASSERT_FALSE(sb->is_connected(NORTH,WEST,0));
  ASSERT_FALSE(sb->is_connected(SOUTH,WEST,0));
  ASSERT_TRUE(sb->is_connected(SOUTH,EAST,0));
  ASSERT_FALSE(sb->is_connected(WEST,EAST,0));
}

TEST(SwitchBlockTest, conn3) {
  switch_block* sb = new switch_block(4);
  
  sb->connect(NORTH, SOUTH, 0);
  sb->connect(SOUTH, EAST,0);
  sb->connect(EAST, WEST,0);
  ASSERT_TRUE(sb->is_connected(NORTH,SOUTH,0));
  ASSERT_TRUE(sb->is_connected(NORTH,EAST,0));
  ASSERT_TRUE(sb->is_connected(NORTH,WEST,0));
  ASSERT_TRUE(sb->is_connected(SOUTH,WEST,0));
  ASSERT_TRUE(sb->is_connected(SOUTH,EAST,0));
  ASSERT_TRUE(sb->is_connected(WEST,EAST,0));
}
