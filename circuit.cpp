#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
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

int circuit::label_h_segment(int x, int y, int t, int label) {
  // h segs: width is grid_size
  vector<int>* cell = h_segs[x + y*grid_size];
  if (get_h_segment(x,y,t)==UNUSED || label == TARGET) {
    (*cell)[t] = label;
    return label;
  } else {
    return get_h_segment(x,y,t);
  }
}

int circuit::label_v_segment(int x, int y, int t, int label) {
  // v segs: width is grid_size+1
  vector<int>* cell = v_segs[x + y*(grid_size+1)];
  if (get_v_segment(x,y,t)==UNUSED || label == TARGET) {
    (*cell)[t] = label;
    return label;
  } else {
    return get_v_segment(x,y,t);
  }
}

int circuit::get_h_segment(int x, int y, int t) {
  // h segs: width is grid_size
  vector<int>* cell = h_segs[x + y*grid_size];
  return (*cell)[t];
}

int circuit::get_v_segment(int x, int y, int t) {
  // v segs: width is grid_size+1
  vector<int>* cell = v_segs[x + y*(grid_size+1)];
  return (*cell)[t];
}

void circuit::allocate_blocks() {

  for(int i = 0; i < (grid_size)*(grid_size+1); ++i) {
    h_segs.push_back(new vector<int>(tracks_per_channel, UNUSED)); // unused
    v_segs.push_back(new vector<int>(tracks_per_channel, UNUSED));
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

enum append_neighbour_result circuit::append_neighbouring_segments(segment* seg, queue<segment*>& exp_list) {
  enum append_neighbour_result rc = NONE_ADDED;
  int next_label = seg->len;

  // this struct helps organizes the test tables below
  struct seg_test_entry {
    int x;
    int y;
    int vert;
  };

  int x = seg->x;
  int y = seg->y;

  struct seg_test_entry vert_tab[] = {
    {x  , y-1,  1},
    {x  , y+1,  1},
    {x-1, y  ,  0},
    {x  , y  ,  0},
    {x-1, y+1,  0},
    {x  , y+1,  0},
  };

  struct seg_test_entry hor_tab[] = {
    {x-1, y  ,  0},
    {x+1, y  ,  0},
    {x  , y-1,  1},
    {x+1, y-1,  1},
    {x  , y  ,  1},
    {x+1, y  ,  1},
  };

  struct seg_test_entry* test_tab = hor_tab;
  if (seg->vert)
    test_tab = vert_tab;

  //assumption: hor_tab and vert_tab are the same size
  for(int i=0; i < sizeof(vert_tab)/sizeof(struct seg_test_entry); ++i) {
    int x = test_tab[i].x;
    int y = test_tab[i].y;

    if (x < 0 || x >= grid_size + (seg->vert ? 1:0))
      continue;

    if (y < 0 || y >= grid_size + (seg->vert ? 0:1)) 
      continue;

    int result;
    if (test_tab[i].vert)
      result = label_v_segment(x,y,seg->track,next_label);
    else
      result = label_h_segment(x,y,seg->track,next_label);

    if (result == next_label) {
      // we successfully claimed this segment (it was unused)
      // add it to the expansion list
      exp_list.push(new segment(x,y,seg->track,test_tab[i].vert,seg->len+1));
      spdlog::debug("added len {} at {}, {}",seg->len+1,x,y);
      rc = SOME_ADDED;
    } else if (result == TARGET) {
      // we have reached a terminal connection
      spdlog::info("TARGET FOUND!!");
      rc = TARGET_FOUND;
      break;
    } 
    //if neither happened - this was a segment that was already used
  }
  return rc;
}

int pin_to_seg_dx(int pin){
  // convert LB pin to either vertical or horizontal segment
  switch(pin){
    case 1: // south
    case 3: // north
    case 4: // west
      return 0;
    case 2: // east
      return 1;
    default:
      spdlog::error("invalid pin, cannot proceed");
      exit(1);
      return 0;
  }
}

int pin_to_seg_dy(int pin){
  // convert LB pin to either vertical or horizontal segment
  switch(pin){
    case 2:
    case 3:
    case 4:
      return 0;
    case 1:
      return 1;
    default:
      spdlog::error("invalid pin, cannot proceed");
      exit(1);
      return 0;
  }
}

int pin_to_seg_vert(int pin){
  // convert LB pin to either vertical or horizontal segment
  switch(pin){
    // east and west - these need to attach to a vert segment
    case 2:
    case 4:
      return 1;
    // south and north - these need to attach to a horiz segment
    case 3:
    case 1:
      return 0;
    default:
      spdlog::error("invalid pin, cannot proceed");
      exit(1);
      return 0;
  }
}

bool circuit::route_conn(connection* conn) {
    int len = 0;
    queue<segment*> exp_list;

    spdlog::debug("routing connection {}", conn->to_string());

    // convert LB pin to either vertical or horizontal segment
    int seg_start_x = conn->x0 + pin_to_seg_dx(conn->p0);
    int seg_start_y = conn->y0 + pin_to_seg_dy(conn->p0);
    int seg_start_vert = pin_to_seg_vert(conn->p0);

    // convert LB pin to either vertical or horizontal segment
    int seg_end_x = conn->x1 + pin_to_seg_dx(conn->p1);
    int seg_end_y = conn->y1 + pin_to_seg_dy(conn->p1);
    int seg_end_vert = pin_to_seg_vert(conn->p1);

    // initial population - source conns
    for (int track = 0; track < tracks_per_channel; ++track) {
      exp_list.push(new segment(seg_start_x,seg_start_y,track,seg_start_vert,0));
    }

    // mark end segs as targets
    for (int track = 0; track < tracks_per_channel; ++track) {
      if(seg_end_vert)
        label_v_segment(seg_end_x, seg_end_y, track, TARGET);
      else
        label_h_segment(seg_end_x, seg_end_y, track, TARGET);
    }

    // now loop 
    while (!exp_list.empty()) {

        segment* seg = exp_list.front();
        spdlog::debug("iterate with seg ({} {} {} {})", seg->x, seg->y, seg->track, (seg->vert ? 'V': 'H'));
        exp_list.pop();
        if (TARGET_FOUND == append_neighbouring_segments(seg, exp_list)) {
          spdlog::debug("segment: {} len", seg->len);
          traceback(seg, exp_list);
          break;
        }
    }

    if (exp_list.empty()) {
      spdlog::error("exp_list empty - no route found");  
      return false;
    } else {
      spdlog::info("successfully routed");  
      return true;
    }
}

void circuit::traceback(segment* end, queue<segment*>& exp_list) {
    // see which neighbour is the lowest cost connection
    int x=end->x;
    int y=end->y;
    int track=end->track;
    int i=0;
    struct helper {
        int val;
        int x;
        int y;
        int vert;
    };
    vector<struct helper> neighbours;
    if (end->vert) {
        int n=UNUSED, ne=UNUSED, nw=UNUSED, s=UNUSED, se=UNUSED, sw=UNUSED;
        n = get_v_segment(x,y-1,track);
        ne = get_h_segment(x-1,y-1,track);
        nw = get_h_segment(x+1,y-1,track);
        s = get_v_segment(x,y+1,track);
        se = get_h_segment(x-1,y+1,track);
        sw = get_h_segment(x+1,y+1,track);

        neighbours.push_back({n , x  , y-1, 1});
        neighbours.push_back({ne, x-1, y-1, 0});
        neighbours.push_back({nw, x+1, y-1, 0});
        neighbours.push_back({s , x  , y+1, 1});
        neighbours.push_back({se, x-1, y+1, 0});
        neighbours.push_back({sw, x+1, y+1, 0});
    }
    else {
        int e=UNUSED, ne=UNUSED, nw=UNUSED, w=UNUSED, se=UNUSED, sw=UNUSED;
        w = get_h_segment(x-1,y,track);
        e = get_h_segment(x+1,y,track);
        ne = get_v_segment(x+1,y-1,track);
        nw = get_v_segment(x-1,y-1,track);
        se = get_v_segment(x+1,y+1,track);
        sw = get_v_segment(x-1,y+1,track);

        neighbours.push_back({e , x  , y-1, 0});
        neighbours.push_back({ne, x-1, y-1, 1});
        neighbours.push_back({nw, x+1, y-1, 1});
        neighbours.push_back({w , x  , y+1, 0});
        neighbours.push_back({se, x-1, y+1, 1});
        neighbours.push_back({sw, x+1, y+1, 1});
    }
    // find min x,y position (and whether thats h or v)
}

bool circuit::route() {

    bool result = true;
    for (auto conn : conns ) {
        result &= route_conn(conn);
    }
    return result;
}
