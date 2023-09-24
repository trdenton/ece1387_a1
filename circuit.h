#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
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

class switch_block {
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
