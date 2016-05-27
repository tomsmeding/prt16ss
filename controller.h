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

	static const unordered_map<string,function<void(Spreadsheet&,SheetView&)>> commands;
	
public:
	SheetController();
	SheetController(string filename);
	
	bool save(); //returns whether successful
	void runloop();
};
