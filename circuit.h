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
    logic_block(int index, int grid_size, int tracks_per_channel) {
      int n_north = tracks_per_channel;
      int n_south = tracks_per_channel;
      int n_east = tracks_per_channel;
      int n_west = tracks_per_channel;

      if (index < grid_size) {
        n_north = 0;
      } 

      // along the bottom row
      if (index > grid_size*(grid_size-1)-1) {
        n_south = 0;
      }

      if (index % grid_size == 0) {
        n_west = 0;
      }

      if (index % grid_size == grid_size-1) {
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
    switch_block(int tracks_per_channel) {
      conns = new vector<uint16_t>(tracks_per_channel, 0UL);
    }

    void connect(enum direction src, enum direction dst, int track) {
      if (track < 0 || track >= conns->size()) {
        spdlog::error("track {} out of range", track);
        return;
      }
      (*conns)[track] |= connect_mask(src,dst);
    }

    bool is_connected(enum direction src, enum direction dst, int track) {
      if (track < 0 || track >= conns->size()) {
        spdlog::error("track {} out of range", track);
        return false;
      }
      uint16_t conn = (*conns)[track];
      cerr << "connval " << hex << conn << endl;
      return track_connected(conn, src, dst);
    }

  private:
    uint16_t connect_mask(enum direction src, enum direction dst) {
    // 64 bit field
    // 1 index selects the nibble (4 nibbles)
    // other index selects the bit within that nibble
    // e.g. 0b0000 0000 0000 0010 indicates north is connected to south
    // but consider that it is bidirectional, so this must actually mean
    // 0b0000 0000 0001 0010
    // 
      uint16_t mask = (1<<src)<<(4*dst) | (1<<dst)<<(4*src);
      return mask;
    }

    bool track_connected(uint16_t conns, enum direction src, enum direction dst) {
    // it can be direct connection, or indirect connection via 1 or 2 intermediates

    // WESN    N
    // WESN    S
    // WESN    E
    // WESN    W

    uint16_t cm = connect_mask(src,dst);

    cerr << "CM " << hex << cm << endl; 

    if (conns & cm)
      return true;

    // no connection yet... try by way of 1 intermediate
    uint16_t cmw = (conns & connect_mask(src,WEST)) && ( conns & connect_mask(WEST,dst));
    uint16_t cme = (conns & connect_mask(src,EAST)) && ( conns & connect_mask(EAST,dst));
    uint16_t cmn = (conns & connect_mask(src,NORTH)) && ( conns & connect_mask(NORTH,dst));
    uint16_t cms = (conns & connect_mask(src,SOUTH)) && ( conns & connect_mask(SOUTH,dst));

    cerr << "cmw cme cmn cms " << hex << cmw << " " << cme << " " << cmn << " " << cms << endl;

    if (cmw || cme || cmn || cms)
      return true;

    // try 2 intermediates
    uint16_t bm = 0UL;

    if (conns & connect_mask(src,NORTH)) {
      bm |= connect_mask(src,NORTH);
    }

    
    /*
    uint16_t cmwe = (conns & connect_mask(src,WEST)) && ( conns & connect_mask(WEST,EAST)) && (conns & connect_mask(EAST,dst));
    uint16_t xxxx = (conns & connect_mask(src,SOUTH)) && ( conns & connect_mask(SOUTH,EAST)) && (conns & connect_mask(EAST,dst));
    uint16_t cmwn = (conns & connect_mask(src,WEST)) && ( conns & connect_mask(WEST,NORTH)) && (conns & connect_mask(NORTH,dst));
    uint16_t cmws = (conns & connect_mask(src,WEST)) && ( conns & connect_mask(WEST,SOUTH)) && (conns & connect_mask(SOUTH,dst));
    uint16_t cmen = (conns & connect_mask(src,EAST)) && ( conns & connect_mask(EAST,NORTH)) && (conns & connect_mask(NORTH,dst));
    uint16_t cmes = (conns & connect_mask(src,EAST)) && ( conns & connect_mask(EAST,SOUTH)) && (conns & connect_mask(SOUTH,dst));
    uint16_t cmns = (conns & connect_mask(src,NORTH)) && ( conns & connect_mask(NORTH,SOUTH)) && (conns & connect_mask(SOUTH,dst));
    */

   if ( cmwe || cmwe || cmws || cmen || cmes || cmns || xxxx)
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
