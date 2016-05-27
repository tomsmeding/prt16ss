#include <iostream>
#include <string>
#include "controller.h"

using namespace std;

int main(int argc,char **argv) {
	SheetController controller(argc>=2 ? argv[1] : "");
	controller.runloop();
	
	return 0;
}
