#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include "spdlog/spdlog.h"
#include "circuit.h"
#include <thread>
#include <unistd.h>
#include <condition_variable>

using namespace std; 

condition_variable cv;
mutex cv_m;
int update_sig = 0;

void circuit_wait_for_ui() {
    unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk,[]{return update_sig==1;});
    update_sig=0;
}

void circuit_next_step() {
    {
        lock_guard<mutex> lk(cv_m);
        update_sig = 1;
    }
    cv.notify_one();
}

circuit::circuit(string file) {
  layers = 1;
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

    while (true) {
      getline(infile, line);
      std::stringstream ss(line);
      istream_iterator<std::string> begin(ss);
      istream_iterator<std::string> end;
      vector<string> vstrings(begin, end);
      if (vstrings[0] == "-1")
        break;
      spdlog::debug("size of vstrings: {}", vstrings.size());
      connection* c = new connection(vstrings);
      spdlog::debug("size of vstrings: {}", vstrings.size());
      if (c->d0 == 1 or c->d1 == 1) {
        spdlog::debug("detected dense circuit");
        layers = 2;
      }
      add_connection(c);
    }
    string conns = dump_connections();
    spdlog::debug("connection dump:\n{}", conns);

    infile.close();

    allocate_blocks();
  } else {
    spdlog::error("Could not open {}", file);
  }
}

bool circuit::label_h_segment(int x, int y, int t, int label) {
  // h segs: width is grid_size
  vector<int>* cell = h_segs[x + y*grid_size];
  if (get_h_segment(x,y,t)==UNUSED || label == TARGET || label == USED) {
    (*cell)[t] = label;
    return true;
  } else {
    return false;
  }
}

bool circuit::label_v_segment(int x, int y, int t, int label) {
  // v segs: width is grid_size+1
  vector<int>* cell = v_segs[x + y*(grid_size+1)];
  if (get_v_segment(x,y,t)==UNUSED || label == TARGET || label == USED) {
    (*cell)[t] = label;
    return true;
  } else {
    return false;
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
    h_segs.push_back(new vector<int>(tracks_per_channel, UNUSED));
    v_segs.push_back(new vector<int>(tracks_per_channel, UNUSED));
  }

  for( int layer = 0; layer < layers; layer++ ) {
      for (int i = 0; i < grid_size*grid_size; ++i) {
          int x = i%grid_size;
          int y = i/grid_size;
          logic_block* lb = new logic_block(x, y, grid_size, tracks_per_channel);
          logic_blocks.push_back(lb);
      }
  }

  for(int i = 0; i < (grid_size+1)*(grid_size+1); ++i) {
    int x = i%(grid_size+1);
    int y = i/(grid_size+1);
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
    int index = x + y*(grid_size+1);
    if (index < switch_blocks.size()) {
        ret = switch_blocks[index];
    }
    return ret;
}

vector<segment* > segment::get_neighbours() {
    vector<segment*> neighbours;

    if (vert) {
        neighbours.push_back(new segment(x  , y-1, track, 1, UNUSED));
        neighbours.push_back(new segment(x  , y+1, track, 1, UNUSED));
        neighbours.push_back(new segment(x-1, y  , track, 0, UNUSED));
        neighbours.push_back(new segment(x  , y  , track, 0, UNUSED));
        neighbours.push_back(new segment(x-1, y+1, track, 0, UNUSED));
        neighbours.push_back(new segment(x  , y+1, track, 0, UNUSED));
    } else {
        neighbours.push_back(new segment(x-1, y  , track, 0, UNUSED));
        neighbours.push_back(new segment(x+1, y  , track, 0, UNUSED));
        neighbours.push_back(new segment(x  , y-1, track, 1, UNUSED));
        neighbours.push_back(new segment(x+1, y-1, track, 1, UNUSED));
        neighbours.push_back(new segment(x  , y  , track, 1, UNUSED));
        neighbours.push_back(new segment(x+1, y  , track, 1, UNUSED));
    }
    
    return neighbours;
}

enum append_neighbour_result circuit::append_neighbouring_segments(segment* seg, queue<segment*>& exp_list) {
  enum append_neighbour_result rc = NONE_ADDED;
  int next_label = seg->len + 1;

  vector<segment* > neighbours = seg->get_neighbours();

  //assumption: hor_tab and vert_tab are the same size
  for( segment* neighbour : neighbours) {
    
    if (!segment_in_bounds(*neighbour))
      continue;

    int old_val = get_seg_label(neighbour);

    bool result = label_segment(neighbour, next_label);

    if (result) {
      // we successfully claimed this segment (it was unused)
      // add it to the expansion list
      neighbour->len = next_label;
      exp_list.push(neighbour);
      spdlog::debug("added len {} at {}, {}",next_label,neighbour->x,neighbour->y);
      rc = SOME_ADDED;
    } else if (old_val == TARGET) {
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
      return 1;
    case 1:
      return 0;
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

bool circuit::route_conn(connection* conn, int track, bool interactive) {
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
    if(seg_start_vert) {
        seg_start_y = conn->y0;
    } else {
        seg_start_x = conn->x0;
    }
    segment* n = new segment(seg_start_x,seg_start_y,track,seg_start_vert,0);
    if (label_segment(n,SOURCE)) { // only attempt the first one
        exp_list.push(n);
    } else {
        delete(n);
        return false;
    }

    segment* end;
    // mark end segs as targets
    // has to be same track as source
    if(seg_end_vert) {
        end = new segment(seg_end_x, conn->y1, track, seg_end_vert, TARGET);
    } else {
        end = new segment(conn->x1, seg_end_y, track, seg_end_vert, TARGET);
    }
    label_segment(end, TARGET);

    // now loop 
    while (!exp_list.empty()) {
        if (interactive) {
            circuit_wait_for_ui();
        }

        segment* seg = exp_list.front();
        spdlog::debug("iterate with seg ({} {} {} {} {})", seg->x, seg->y, seg->track, seg->len, (seg->vert ? 'V': 'H'));
        exp_list.pop();
        if (TARGET_FOUND == append_neighbouring_segments(seg, exp_list)) {
            traceback(end, interactive);
            connect_lb(conn, end);
            break;
        }
    }

    if (exp_list.empty()) {
        spdlog::error("could not route design");  
        return false;
    } else {
        spdlog::info("successfully routed design");  
        return true;
    }
}

bool circuit::segment_in_bounds(segment& in) {
    if (in.vert) {
        if (in.x >= 0 && in.x <= grid_size && in.y >=0 && in.y < grid_size)
            return true;
    } else {
        if (in.x >= 0 && in.x < grid_size && in.y >=0 && in.y <= grid_size)
            return true;
    }
    return false;
}

int circuit::get_seg_label(segment* a) {
    if (a->vert)
        return get_v_segment(a->x,a->y,a->track);
    return get_h_segment(a->x,a->y,a->track);
}

bool circuit::label_segment(segment* a, int label) {
    if (a->vert)
        return label_v_segment(a->x,a->y,a->track,label);
    return label_h_segment(a->x,a->y,a->track,label);
}

void circuit::clean_up_unused_segments_1d(vector<vector<int>*>& segs, bool clean_target, bool clean_source) {
    for(auto h: segs) {
        for(auto& t: *h) {
            if (t != USED && t != TARGET && t != SOURCE)
                t = UNUSED;
            if (clean_target && t == TARGET)
                t = UNUSED;
            if (clean_source && t == SOURCE)
                t = UNUSED;
        }
    }
}

void circuit::clean_up_unused_segments(bool clean_target, bool clean_source) {
    clean_up_unused_segments_1d(h_segs,clean_target,clean_source);
    clean_up_unused_segments_1d(v_segs,clean_target,clean_source);
}

void circuit::connect_sb(segment* s1, segment* s2) {
    // what is the relation betweent the two segments?
    // there are 4 possible scenarios
    int config = (((s1->vert)<<1) | s2->vert);
    const int HH = 0;
    const int HV = 1;
    const int VH = 2;
    const int VV = 3;

    switch_block* sb;
    enum direction src;
    enum direction dst;
    int track = s1->track;
    int switch_x = 0;
    int switch_y = 0;
    char d;
    switch(config) {
        case HH:
            src = EAST;
            dst = WEST;
            switch_x = max(s1->x, s2->x);
            switch_y = s1->y;
            d = 'H';
            break;
        case VH:    
            switch_x = s1->x;
            switch_y = s2->y;
            if (s1->y == s2->y) {   //s1 is above s2
                dst = NORTH;
            } else {
                dst = SOUTH;
            }
            if (s1->x == s2->x) {
                src = EAST;
            } else {
                src = WEST;
            }
            break;
        case HV:
            switch_x = s2->x;
            switch_y = s1->y;
            if (s1->y == s2->y) {   //s1 is above s2
                dst = NORTH;
            } else {
                dst = SOUTH;
            }
            if (s1->x == s2->x) {
                src = EAST;
            } else {
                src = WEST;
            }
            break;
        case VV:
            src = NORTH;
            dst = SOUTH;
            switch_x = s1->x;
            switch_y = max(s1->y, s2->y);
            d = 'V';
            break;
        default:
            spdlog::error("bad sb connect config... this shouldnt happen.  See ya");
            exit(1);
            break;
    }
    spdlog::debug("connecting switch {}, {} {}", switch_x, switch_y, d);
    sb = get_switch_block(switch_x,switch_y);
    sb->connect(src,dst,track);
    
}

void circuit::connect_lb(connection* conn, segment* seg) {
    // should just need:
    //  x0,y0,p0,  x1,y1,p1
    //  the track we are routing on (available from segment)
    logic_block* lb0 = get_logic_block(conn->x0, conn->y0); 
    logic_block* lb1 = get_logic_block(conn->x1, conn->y1); 
    int track = seg->track;

    // HACK: north and south conns are swapped.  if it gets used somewhere else, fix it.
    switch(conn->p0) {
        case 1:
            (*lb0->north_conns)[(tracks_per_channel-1)-track] = 1;
        break;
        case 2:
            (*lb0->east_conns)[track] = 1;
        break;
        case 3:
            (*lb0->south_conns)[track] = 1;
        break;
        case 4:
            (*lb0->west_conns)[(tracks_per_channel-1) - track] = 1;
        break;
    }
    switch(conn->p1) {
        case 1:
            (*lb1->north_conns)[(tracks_per_channel-1)-track] = 1;
        break;
        case 2:
            (*lb1->east_conns)[track] = 1;
        break;
        case 3:
            (*lb1->south_conns)[track] = 1;
        break;
        case 4:
            (*lb1->west_conns)[(tracks_per_channel-1) - track] = 1;
        break;
    }
}

void circuit::traceback(segment* seg, bool interactive) {
    segment* next;
    segment* cur = seg;
    while( traceback_find_next(cur,next) > 0  && get_seg_label(cur)!=0) {
        if (interactive)
            circuit_wait_for_ui();
        label_segment(cur,USED);
        connect_sb(cur,next);
        cur = next;
    }

    label_segment(seg, USED); // this should be the end one
    label_segment(cur, USED); // and this one should be the beginning
    // final one
    clean_up_unused_segments(true,true);
}

int circuit::traceback_find_next(segment* end, segment*& found) {
    // see which neighbour is the lowest cost connection
    // return # of neighbours (to accomodate zero return)

    class circuit_seg_pair {
        public:
            circuit* circ;
            segment* seg;
        circuit_seg_pair(circuit* c, segment* s) {
            circ = c;
            seg = s;
        }
    };
    vector<segment*> tests = end->get_neighbours();
    vector<circuit_seg_pair*> neighbours;

    for (auto test: tests) {
        if (segment_in_bounds(*test)) {
            int val = get_seg_label(test);
            if (ON_PATH(val)) {
                spdlog::debug("neighbour has passed the test: {} @ ({},{} ({}))",val, test->x, test->y, test->vert ? 'V':'H');
                neighbours.push_back(new circuit_seg_pair(this,test));
            }
        }
    }
    if (neighbours.size() > 0) {
        spdlog::debug("We have found {} neighbours", neighbours.size());
        circuit_seg_pair* smallest = *std::min_element(neighbours.begin(),neighbours.end(),
                [](circuit_seg_pair* a, circuit_seg_pair* b){return a->circ->get_seg_label(a->seg) < b->circ->get_seg_label(b->seg);}
                );
        spdlog::debug("the smallest is {}", get_seg_label(smallest->seg));
        found = smallest->seg;
    }
    return neighbours.size();
}

bool circuit::route(bool interactive) {

    bool result = true;
    int track = 0;
    for (auto conn : conns ) {
        bool eventually_routed = false;
        for(int i = 0; i < tracks_per_channel; ++i) {
            track = (track + 1) % tracks_per_channel;
            eventually_routed |= route_conn(conn, track, interactive);
            if (eventually_routed)
                break;
        }
        result &= eventually_routed;
    }
    return result;
}
