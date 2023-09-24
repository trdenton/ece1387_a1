#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <getopt.h>
#include "spdlog/spdlog.h"
#include "version.h"
#include "circuit.h"

using namespace std;


circuit* read_input(string file) {
  string line;
  int grid;
  int tracks_per_channel;
  circuit* circ = nullptr;
  ifstream infile (file);
  spdlog::debug("Reading input file {}", file);
  if (infile.is_open()) {
    // first line -  grid size 
    // second line - tracks per channel

    getline(infile, line);
    grid = stoi(line);

    getline(infile, line);
    tracks_per_channel = stoi(line);

    spdlog::debug("grid size {} tracks per channel {}", grid, tracks_per_channel);
    circ = new circuit(grid, tracks_per_channel);

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
        circ->add_connection(conn);
      }
    }
    string conns = circ->dump_connections();
    spdlog::debug("connection dump:\n{}", conns);

    infile.close();
  }
  return circ;
}

void print_usage() {
  cout << "Usage: ./a1 [-hv] [-d] -f circuit_file" << endl;
  cout << "\t-h: this help message" <<endl;
  cout << "\t-v: print version info" <<endl;
  cout << "\t-f circuit_file: the circuit file (required)" <<endl;
  cout << "\t-d: turn on debug log level" <<endl;
}

int main(int n, char** args) {
  string file = "";

  for(;;)
  {
    switch(getopt(n, args, "vhf:d"))
    {
      case 'f':
        file = optarg;
        continue;

      case 'd':
        spdlog::set_level(spdlog::level::debug);
        continue;

      case 'v':
        spdlog::info("a1 - Troy Denton 2023");
        spdlog::info("Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
        spdlog::info("Commit {}", GIT_COMMIT);
        spdlog::info("Built {}" , __TIMESTAMP__);
        return 0;

      case '?':
      case 'h':
      default :
        print_usage();
        return 1;

      case -1:
        break;
    }
    break;
  }

  if (file == "") {
    cerr << "Error: must provide input file" << endl;
    print_usage();
    return 1;
  }

  spdlog::info("a1 - Troy Denton 2023");
  spdlog::info("Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
  spdlog::info("Commit {}", GIT_COMMIT);
  spdlog::info("Built {}" , __TIMESTAMP__);

  circuit* circ = read_input(file);

  spdlog::info("Exiting");
  delete(circ);
  return 0;
}
