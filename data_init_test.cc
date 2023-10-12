#include <gtest/gtest.h>
#include <vector>
#include "circuit.h"

/*

[]=[]=[]=[]
||x||x||x||
[]=[]=[]=[]
||x||x||x||
[]=[]=[]=[]
||x||x||x||
[]=[]=[]=[]

*/
TEST(BlockInit, cct1) {
  circuit* c = new circuit("../data/cct1");
  
  EXPECT_EQ(c->logic_blocks.size(), 16);
  EXPECT_EQ(c->switch_blocks.size(), 25);

  // top left corner
  EXPECT_EQ(c->logic_blocks[0]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[0]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[0]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[0]->west_conns->size(), 8);

  // top edge
  EXPECT_EQ(c->logic_blocks[1]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[1]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[1]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[1]->west_conns->size(), 8);

  // bottom right corner
  EXPECT_EQ(c->logic_blocks[15]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[15]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[15]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[15]->west_conns->size(), 8);

  // top right corner
  EXPECT_EQ(c->logic_blocks[3]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[3]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[3]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[3]->west_conns->size(), 8);

  // left edge
  EXPECT_EQ(c->logic_blocks[4]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[4]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[4]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[4]->west_conns->size(), 8);

  // right edge
  EXPECT_EQ(c->logic_blocks[7]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[7]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[7]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[7]->west_conns->size(), 8);

  // bottom edge
  EXPECT_EQ(c->logic_blocks[13]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[13]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[13]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[13]->west_conns->size(), 8);

  // bottom left corner
  EXPECT_EQ(c->logic_blocks[12]->north_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[12]->south_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[12]->east_conns->size(), 8);
  EXPECT_EQ(c->logic_blocks[12]->west_conns->size(), 8);

  delete(c);
}

TEST(BlockInit, cct2) {
  circuit* c = new circuit("../data/cct2");
  
  EXPECT_EQ(c->logic_blocks.size(), 6*6);
  EXPECT_EQ(c->switch_blocks.size(), 7*7);

  delete(c);
}

TEST(BlockInit, cct3) {
  circuit* c = new circuit("../data/cct3");
  
  EXPECT_EQ(c->logic_blocks.size(), 14*14);
  EXPECT_EQ(c->switch_blocks.size(), 15*15);

  delete(c);
}
