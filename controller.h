#include "spreadsheet.h"
#include "view.h"
#include <string>

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
