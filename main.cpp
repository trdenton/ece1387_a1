#include <iostream>
#include "version.h"

using namespace std;

int main(int n, char** args) {
	cout << "a1 - Troy Denton 2023" << endl;
	cout << "Version " << VERSION_MAJOR << "." << VERSION_MINOR << endl;
	cout << "Commit " << GIT_COMMIT << endl;
	cout << "Built " << __TIMESTAMP__ << endl;
	return 0;
}
