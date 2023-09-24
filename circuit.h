#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <list>
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

class circuit {
  public:
    int grid_size, tracks_per_channel;
    list<connection*> conns;

    circuit(int gs, int tpc) {
      grid_size = gs;
      tracks_per_channel = tpc;
    }

    ~circuit() {
      for (auto* conn : conns){
        delete(conn);
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
};
#endif
