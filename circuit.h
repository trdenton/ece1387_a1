#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <inttypes.h>
#include "spdlog/spdlog.h"
using namespace std;
class connection {
  public:
    int x0 = 0, y0 = 0, p0 = 0, x1 = 0, y1 = 0, p1 = 0;
    connection(int x_0, int y_0, int p_0, int x_1, int y_1, int p_1) {
      x0 = x_0;
      y0 = y_0;
      p0 = p_0;
      x1 = x_1;
      y1 = y_1;
      p1 = p_1;
    }

    string to_string() {
      ostringstream outstring;
      outstring << "x0: " << x0 << " y0: " << y0 << " p0: " << p0 << " x1: " << x1 << " y1: " << y1 << " p1: " << p1;
      return outstring.str();
    }
};

class logic_block {
  public:
    vector<char>* north_conns;
    vector<char>* south_conns;
    vector<char>* east_conns;
    vector<char>* west_conns;
    int x;
    int y;
    logic_block(int _x, int _y, int grid_size, int tracks_per_channel) {
      x = _x;
      y = _y;
      int n_north = tracks_per_channel;
      int n_south = tracks_per_channel;
      int n_east = tracks_per_channel;
      int n_west = tracks_per_channel;

      if (y == 0) {
        n_north = 0;
      } 

      // along the bottom row
      if (y == grid_size-1) {
        n_south = 0;
      }

      if (x == 0 ) {
        n_west = 0;
      }

      if (x == grid_size-1) {
        n_east = 0;
      }

      north_conns = new vector<char>(n_north);
      south_conns = new vector<char>(n_south);
      east_conns = new vector<char>(n_east);
      west_conns = new vector<char>(n_west);
    }

    ~logic_block() {
      delete(north_conns);
      delete(south_conns);
      delete(east_conns);
      delete(west_conns);
    }
};

enum direction {
  NORTH=0,
  SOUTH,
  EAST,
  WEST,
  N_DIRECTIONS
};

class switch_block {
  vector<uint16_t>* conns;

  public:
  int x;
  int y;
  switch_block(int _x, int _y, int tracks_per_channel) {
    x = _x;
    y = _y;
    conns = new vector<uint16_t>(tracks_per_channel, 0UL);
  }

  void connect(enum direction src, enum direction dst, int track) {
    if (track < 0 || track >= conns->size()) {
      spdlog::error("track {} out of range", track);
      return;
    }
    (*conns)[track] |= (1<<src)<<(4*dst) | (1<<dst)<<(4*src);
  }

  bool is_connected(enum direction src, enum direction dst, int track) {
    if (track < 0 || track >= conns->size()) {
      spdlog::error("track {} out of range", track);
      return false;
    }
    uint16_t conn = (*conns)[track];
    return track_connected(conn, src, dst);
  }

  private:

  bool track_connected(uint16_t conns, enum direction src, enum direction dst) {
    // it can be direct connection, or indirect connection via 1 or 2 intermediates

    // WESN    N
    // WESN    S
    // WESN    E
    // WESN    W

    uint16_t src_conns = (conns>>(4*src))&0x00ff;


    for(int i = 0; i < N_DIRECTIONS; ++i) {
      if (src_conns & (1<<i)) {
        src_conns |= ((conns >> (4*i)) & 0x00ff);
      }
    }

    if (src_conns & (1<<dst))
      return true;

    return false;
  }

};

class circuit {
  public:
    int grid_size, tracks_per_channel;
    vector<connection*> conns;
    vector<logic_block*> logic_blocks;
    vector<switch_block*> switch_blocks;

    circuit(string f);

    ~circuit() {
      for (auto* conn : conns){
        delete(conn);
      }

      for (auto* sb : switch_blocks){
        delete(sb);
      }

      for (auto* lb : logic_blocks){
        delete(lb);
      }
    }

    void add_connection(connection* conn) {
      conns.push_back(conn);
    }

    string dump_connections() {
      ostringstream outstring;
      for (auto* conn : conns){
        outstring << conn->to_string() << endl;
      }

      return outstring.str();
    }

    void allocate_blocks();

};
#endif
