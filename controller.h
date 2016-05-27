// Tom Smeding (s1685694), Tim Brouwer (s1663615), Ruben Turkenburg (s1659685)

#pragma once

#include "spreadsheet.h"
#include "view.h"
#include <string>

/*
The class that does the I/O and connects the model and the view together.
*/

class SheetController{
private:
	Spreadsheet sheet;
	SheetView view;
	string fname;
	
public:
	SheetController();
	SheetController(string filename);
	
	void save();
	void runloop();
};