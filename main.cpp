#include <iostream>
#include <fstream>
#include <string>
#include "spdlog/spdlog.h"
#include "version.h"

using namespace std;


int read_input(string file) {
	string line;
	int grid;
	int tracks_per_channel;
	ifstream infile (file);
	spdlog::debug("Reading input file {}", file);
	if (infile.is_open()) {
		// first line -  grid size 
		// second line - tracks per channel

		getline(infile, line);
		grid = stoi(line);

		getline(infile, line);
		tracks_per_channel = stoi(line);

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
		}


		infile.close();
	}
	return 0;
}

int main(int n, char** args) {
	spdlog::info("a1 - Troy Denton 2023");
	spdlog::info("Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
	spdlog::info("Commit {}", GIT_COMMIT);
	spdlog::info("Built {}" , __TIMESTAMP__);


	read_input("cct1");
	spdlog::info("Exiting");
	return 0;
}
