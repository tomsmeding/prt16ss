#include <iostream>
#include <string>
#include "controller.h"

using namespace std;

int main() {
	char load;
	string filename = "";
	cout << "Would you like to load a file? (y/n)" << endl;
	cin >> load;
	if (load == 'y') {
		cout << "Filename to load from:" << endl;
		cin >> filename;	
	}
	SheetController controller(filename);
	controller.runloop();
	
	return 0;
}
