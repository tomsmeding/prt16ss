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

	enum CommandRet {
		CR_OK,
		CR_FAIL,
		CR_CANCELLED,
		CR_QUIT
	};

	static const unordered_map<string,function<CommandRet(SheetController&)>> commands;

public:
	SheetController();
	SheetController(string filename);

	void runloop();
};
