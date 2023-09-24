#include <gtest/gtest.h>
#include <list>
#include "circuit.h"

// Basic file read sanity checks
TEST(FileRead, cct1) {
  circuit* c = new circuit("../data/cct1");

  EXPECT_EQ(c->grid_size, 4);
  EXPECT_EQ(c->tracks_per_channel, 8);
  EXPECT_EQ(c->conns.size(), 11);

  delete(c);
}

TEST(FileRead, cct2) {
  circuit* c = new circuit("../data/cct2");

  EXPECT_EQ(c->grid_size, 6);
  EXPECT_EQ(c->tracks_per_channel, 10);
  EXPECT_EQ(c->conns.size(), 28);

  delete(c);
}

TEST(FileRead, cct3) {
  circuit* c = new circuit("../data/cct3");

  EXPECT_EQ(c->grid_size, 14);
  EXPECT_EQ(c->tracks_per_channel, 14);
  EXPECT_EQ(c->conns.size(), 46);

  delete(c);
}

TEST(FileRead, cct4) {
  circuit* c = new circuit("../data/cct4");

  EXPECT_EQ(c->grid_size, 20);
  EXPECT_EQ(c->tracks_per_channel, 12);
  EXPECT_EQ(c->conns.size(), 135);

  delete(c);
}
