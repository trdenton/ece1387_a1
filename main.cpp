#include <iostream>
#include <fstream>
#include <string>
#include "version.h"

using namespace std;


int read_input(string file) {
	string line;
	int grid;
	int tracks_per_channel;
	ifstream infile (file);
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
			cout << "x0 " << x0 << " y0 " << y0 << " p0 " << p0 << " x1 " << x1 << " y1 " << y1 << " p1 " << p1 << endl;

			all_neg1 = (x0 == -1) && (y0 == -1) && (p0 == -1) && (x1 == -1) && (y1 == -1) && (p1 == -1);
		}


		infile.close();
	}
	return 0;
}

int main(int n, char** args) {
	cout << "a1 - Troy Denton 2023" << endl;
	cout << "Version " << VERSION_MAJOR << "." << VERSION_MINOR << endl;
	cout << "Commit " << GIT_COMMIT << endl;
	cout << "Built " << __TIMESTAMP__ << endl;


	read_input("cct1");
	return 0;
}
