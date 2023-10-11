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

char circuit::label_h_segment(int x, int y, int t, char label) {
  // h segs: width is grid_size
  vector<char>* cell = h_segs[x + y*grid_size];
  if (get_h_segment(x,y,t)=='0') {
    (*cell)[t] = label;
    return label;
  } else {
    return get_h_segment(x,y,t);
  }
}

char circuit::label_v_segment(int x, int y, int t, char label) {
  // h segs: width is grid_size
  vector<char>* cell = v_segs[x + y*(grid_size+1)];
  if (get_v_segment(x,y,t)=='0') {
    (*cell)[t] = label;
    return label;
  } else {
    return get_v_segment(x,y,t);
  }
}

char circuit::get_h_segment(int x, int y, int t) {
  // h segs: width is grid_size
  vector<char>* cell = h_segs[x + y*grid_size];
  if (get_h_segment(x,y,t)=='0')
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

switch_block* circuit::get_switch_block(int x, int y) {
    
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
        int vert;

    route_step(int _x, int _y, int _track, int _vert) {
        x = _x;
        y = _y;
        track = _track;
        vert = _vert;
    }
};

bool circuit::route_conn(connection* conn) {
    queue<route_step*> exp_list;

    spdlog::debug("routing connection {}", conn->to_string());

    // populate initial expansion list
    // what info do i need to do this?

    // if we are on the west pin(4),
    //    vertical seg @ x,y
    // if we are on the east pin(2),
    // if we are on the north pin(3),
    //    use horizontal seg @ y
    // if we are on the south pin(1),
    //    use horizontal seg @ y+1


    // convert LB pin to either vertical or horizontal segment
    int seg_x=conn->x0;
    int seg_y=conn->y0;
    int vert=0;
    switch(conn->p0){
      case 1: // south
        ++seg_y;
        break;
      case 2: // east
        vert=1;
        ++seg_x;
        break;
      case 3: // north
        break;
      case 4: // west
        vert=1;
        break;
      default:
        spdlog::error("invalid pin, cannot proceed");
        exit(1);
        break;
    }
    
    char next_label = '0' + len;

    // this struct helps organizes the test tables below
    struct seg_test_entry {
      int x;
      int y;
      char (*fn)(int,int,int,char);
    };

    struct seg_test_entry vert_tab[] = {
      {x  , y-1,  label_v_seg},
      {x  , y+1,  label_v_seg},
      {x-1, y  ,  label_h_seg},
      {x  , y  ,  label_h_seg},
      {x-1, y+1,  label_h_seg},
      {x  , y+1,  label_h_seg},
    };

    struct seg_test_entry hor_tab[] = {
      {x-1, y  ,  label_h_seg},
      {x+1, y  ,  label_h_seg},
      {x  , y-1,  label_v_seg},
      {x+1, y-1,  label_v_seg},
      {x  , y  ,  label_v_seg},
      {x+1, y  ,  label_v_seg},
    };

    struct seg_test_entry** test_tab = &hor_tab;
    if (vert)
      test_tab = &vert_tab;

    //assumption: hor_tab and vert_tab are the same size
    for(int i=0; i < sizeof(vert_tab)/sizeof(struct seg_test_entry); ++i) {
      int x = (*test_tab)[i].x;
      int y = (*test_tab)[i].y;
      char result = (*test_tab)[i].fn(x,y,track,next_label);
      if (result == next_label) {
        // we successfully claimed this segment (it was unused)
        // add it to the expansion list
        exp_list.push_back(new route_step(x,y,track,vert));
      } else if (result == 'T') {
        // we have reached a terminal connection
        spdlog::info("TERMINUS FOUND!!");
      } 
      //if neither happened - this was a segment that was already used
    }

    // now loop 
    while (!exp_list.empty()) {
        // get the 
        route_step* rs = exp_list.front();
        
          
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
