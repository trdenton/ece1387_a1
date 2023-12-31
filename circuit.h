#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <string>
#include <iostream>
#include <queue>
#include <sstream>
#include <inttypes.h>
#include <limits.h>
#include "spdlog/spdlog.h"

//this is used to instantiate test connections for ui test purposes
#include "ui_tests.h"

using namespace std;

#define UNUSED (INT_MAX)
#define TARGET (INT_MAX-1)
#define USED (INT_MAX-2)
#define ON_PATH(__X__) (__X__ < USED)
#define SOURCE (0)

enum append_neighbour_result {
  NONE_ADDED,
  SOME_ADDED,
  TARGET_FOUND,
};

class segment {
    public:
        int x;
        int y;
        int track;
        int vert;
        int len;

    segment(int _x, int _y, int _track, int _vert, int _len) {
        x = _x;
        y = _y;
        track = _track;
        vert = _vert;
        len = _len;
    }
    vector<segment*> get_neighbours();
};

// connection represents a logical connection
class connection {
  public:
    int x0 = 0, y0 = 0, p0 = 0, x1 = 0, y1 = 0, p1 = 0, d0 = 0, d1 = 0;
    connection(vector<string> toks) {
      if (toks.size() == 6) {
          spdlog::debug("normal circuit");
          x0 = stoi(toks[0]);
          y0 = stoi(toks[1]);
          p0 = stoi(toks[2]);
          x1 = stoi(toks[3]);
          y1 = stoi(toks[4]);
          p1 = stoi(toks[5]);
          // these are the density bits
          d0 = 0;
          d1 = 0;
      } else if (toks.size() == 8) {
          spdlog::debug("dense circuit");
          x0 = stoi(toks[0]);
          y0 = stoi(toks[1]);
          d0 = toks[2] == "a" ? 0 : 1;
          p0 = stoi(toks[3]);

          x1 = stoi(toks[4]);
          y1 = stoi(toks[5]);
          d1 = toks[6] == "a" ? 0 : 1;
          p1 = stoi(toks[7]);
      } else {
        spdlog::debug("UNKNOWN: {}", toks.size());
        exit(1);
      }
    }

    string to_string() {
      ostringstream outstring;
      outstring << "x0: " << x0 << " y0: " << y0 << " p0: " << p0 << " x1: " << x1 << " y1: " << y1 << " p1: " << p1;
      return outstring.str();
    }
};

class logic_block {
  public:
    vector<int>* north_conns;
    vector<int>* south_conns;
    vector<int>* east_conns;
    vector<int>* west_conns;
    int x;
    int y;
    int layer;
    int tracks_per_channel;
    logic_block(int _x, int _y, int _layer, int grid_size, int _tracks_per_channel) {
      x = _x;
      y = _y;
      layer = _layer;
      tracks_per_channel = _tracks_per_channel;

      int n_init = UNUSED;
      int s_init = UNUSED;
      int e_init = UNUSED;
      int w_init = UNUSED;
      
      #ifdef UI_TEST_LB_N_CONN
      n_init = 1;
      #endif

      #ifdef UI_TEST_LB_S_CONN
      s_init = 1;
      #endif

      #ifdef UI_TEST_LB_E_CONN
      e_init = 1;
      #endif

      #ifdef UI_TEST_LB_W_CONN
      w_init = 1;
      #endif

      north_conns = new vector<int>(tracks_per_channel, n_init);
      south_conns = new vector<int>(tracks_per_channel, s_init);
      east_conns = new vector<int>(tracks_per_channel, e_init);
      west_conns = new vector<int>(tracks_per_channel, w_init);
    }

    ~logic_block() {
      delete(north_conns);
      delete(south_conns);
      delete(east_conns);
      delete(west_conns);
    }

    vector<int>* get_pin_conns(int pin) {
      switch(pin) {
        case(1):
          return south_conns;
        case(2):
          return east_conns;
        case(3):
          return north_conns;
        case(4):
          return west_conns;
        default:
          spdlog::error("bad pin: {}, cannot proceed", pin);
          exit(1);
          return nullptr;
      } 
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
  int tracks_per_channel;
  switch_block(int _x, int _y, int _tracks_per_channel) {
    tracks_per_channel = _tracks_per_channel;
    x = _x;
    y = _y;
    conns = new vector<uint16_t>(tracks_per_channel, 0UL);

    #ifdef UI_TEST_SB_N_S
    connect(NORTH,SOUTH,0);
    #endif

    #ifdef UI_TEST_SB_E_W
    connect(EAST,WEST,0);
    #endif

    #ifdef UI_TEST_SB_N_E
    connect(NORTH,EAST,1);
    #endif

    #ifdef UI_TEST_SB_N_W
    connect(NORTH,WEST,1);
    #endif

    #ifdef UI_TEST_SB_S_W
    connect(WEST,SOUTH,1);
    #endif

    #ifdef UI_TEST_SB_S_E
    connect(EAST,SOUTH,1);
    #endif
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

  bool direct_connected(enum direction src, enum direction dst, int track) {
    if (track < 0 || track >= conns->size()) {
      spdlog::error("track {} out of range", track);
      return false;
    }
    uint16_t conn = (*conns)[track];
    return (conn & ((1<<src)<<(4*dst) | (1<<dst)<<(4*src)));
  }

  bool segment_used(enum direction src, int track) {
    if (track < 0 || track >= conns->size()) {
      spdlog::error("track {} out of range", track);
      return false;
    }
    uint16_t conn = (*conns)[track];
    return ( ((conn>>(4*src))&0x000f) != 0);
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
    int layers;
    int grid_size, tracks_per_channel;
    vector<connection*> conns;
    vector<logic_block*> logic_blocks;
    vector<switch_block*> switch_blocks;
    vector<vector<int>*> h_segs;
    vector<vector<int>*> v_segs;

    circuit(string f, int force_w);
    circuit(string f) : circuit(f,0) {};

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

    logic_block* get_logic_block(int x, int y, int layer);
    switch_block* get_switch_block(int x, int y);

    void traceback(segment* end, bool interactive);
    bool route(bool interactive);
    int get_h_segment(int x, int y, int t);
    int get_v_segment(int x, int y, int t);
    int get_seg_label(segment* a);
    bool label_segment(segment* a, int label);
    void clean_up_unused_segments(bool clean_target, bool clean_source);
    int total_segments();
private:
    bool label_h_segment(int x, int y, int t, int label);
    bool label_v_segment(int x, int y, int t, int label);
    void allocate_blocks();
    bool route_conn(connection* conn, int track, bool interactive);
    enum append_neighbour_result append_neighbouring_segments(segment* seg, queue<segment*>& exp_list);
    bool segment_in_bounds(struct segment& in);
    void map_routing_to_ui();
    int traceback_find_next(segment* end, segment*& found);
    void clean_up_unused_segments_1d(vector<vector<int>*>& segs, bool clean_target, bool clean_source);
    int count_used_segs_1d(vector<vector<int>*>& segs);
    void connect_sb(segment* a, segment* b);
    void connect_lb(connection* conn, segment* seg);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
