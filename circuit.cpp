#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "spdlog/spdlog.h"
#include "circuit.h"

using namespace std; 

circuit::circuit(string file) {
  string line;
  ifstream infile (file);
  spdlog::debug("Reading input file {}", file);


  if (infile.is_open()) {

    // first line -  grid size 
    // second line - tracks per channel
    getline(infile, line);
    grid_size = stoi(line);

    getline(infile, line);
    tracks_per_channel = stoi(line);

    spdlog::debug("grid size {} tracks per channel {}", grid_size, tracks_per_channel);

    // read until we get the final line of all -1's
    int x0 = 0, y0 = 0, p0 = 0, x1 = 0, y1 = 0, p1 = 0;
    int all_neg1 = 0;
    while (!all_neg1) {
      infile >> x0;
      infile >> y0;
      infile >> p0;
      infile >> x1;
      infile >> y1;
      infile >> p1;
      spdlog::debug("read in x0 {} y0 {} p0 {} x1 {} y1 {} p1 {}", x0, y0, p0, x1, y1, p1);

      all_neg1 = (x0 == -1) && (y0 == -1) && (p0 == -1) && (x1 == -1) && (y1 == -1) && (p1 == -1);
      if (!all_neg1) {
        connection* conn = new connection(x0,y0,p0,x1,y1,p1);
        add_connection(conn);
      }
    }
    string conns = dump_connections();
    spdlog::debug("connection dump:\n{}", conns);

    infile.close();

    allocate_blocks();
  } else {
    spdlog::error("Could not open {}", file);
  }
}

char circuit::get_h_segment(int x, int y, int t) {
  // h segs: width is grid_size
  vector<char>* cell = h_segs[x + y*grid_size];
  return (*cell)[t];
}

char circuit::get_v_segment(int x, int y, int t) {
  // v segs: width is grid_size+1
  vector<char>* cell = v_segs[x + y*(grid_size+1)];
  return (*cell)[t];
}

void circuit::allocate_blocks() {

  for(int i = 0; i < (grid_size)*(grid_size+1); ++i) {
    h_segs.push_back(new vector<char>(tracks_per_channel, '0'));
    v_segs.push_back(new vector<char>(tracks_per_channel, '0'));
  }

  for(int i = 0; i < grid_size*grid_size; ++i) {
    int x = i%grid_size;
    int y = i/grid_size;
    logic_block* lb = new logic_block(x, y, grid_size, tracks_per_channel);
    logic_blocks.push_back(lb);
  }

  for(int i = 0; i < (grid_size-1)*(grid_size-1); ++i) {
    int x = i%(grid_size-1);
    int y = i/(grid_size-1);
    switch_blocks.push_back(new switch_block(x, y, tracks_per_channel));
  }
}

logic_block* circuit::get_logic_block(int x, int y) {
    
    logic_block* ret = nullptr;
    int index = x + y*grid_size;
    if (index < logic_blocks.size()) {
        ret = logic_blocks[index];
    }
    return ret;
}

logic_block* circuit::get_switch_block(int x, int y) {
    
    switch_block* ret = nullptr;
    int index = x + y*grid_size;
    if (index < switch_blocks.size()) {
        ret = switch_blocks[index];
    }
    return ret;
}

class route_step {
    public:
        int x;
        int y;
        int track;

    route_step(int _x, int _y, int _track) {
        x = _x;
        y = _y;
        track = _track;
    }
};

bool circuit::route_conn(connection* conn) {
    queue<route_step*> exp_list;
    logic_block* start = get_logic_block(conn->x0, conn->y0);
    logic_block* end   = get_logic_block(conn->x1, conn->y1);

    spdlog::debug("routing connection {}", conn->to_string());

    // populate initial expansion list
    // what info do i need to do this?

    vector<char>* tracks = start->get_pin_conns(conn->p0);
    int dist = 0;
    for (int i=0; i < tracks->size(); ++i) {
        if ((*tracks)[i] == '\0') {
            (*tracks)[i] = '0'+(char)i;
            //spdlog::debug("pushed x {} y {} i {}",start->x,start->y,i);
            exp_list.push( new route_step(start->x, start->y, i) );
        }
    }


    // now loop 
    while (!exp_list.empty()) {
        // get the 
        route_step* rs = exp_list.front();
        
        logic_block* lb = get_logic_block(rs->x, rs->y);
          
        exp_list.pop();
        //spdlog::debug("routed x {} y {} i {}",rs->x,rs->y,rs->track);
    }

    return true;
}

bool circuit::route() {

    bool result = true;
    for (auto conn : conns ) {
        result &= route_conn(conn);
    }
    return result;
}
