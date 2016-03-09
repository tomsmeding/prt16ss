#include "types.h"

class Spreadsheet{
public:
	Errtype saveToDisk(string fname); //asks the model to save itself to the specified file
	ErrorOr<string> loadFromDisk(string fname); //asks the model to load itself from the specified file

	Maybe<string> getCellDisplayString(CellAddress addr); //gets string for that cell for display in the sheet;
	                                                      //(Nothing if out of bounds)
	Maybe<string> getCellEditString(CellAddress addr); //gets string containing the raw cell data (for editing);
	                                                   //(Nothing if out of bounds)

	using ChangedList = vector<CellAddress>;
	ChangedList changeCellValue(CellAddress addr,string repr);
	  //changes the raw cell data of a cell, returns list of cells changed in sheet (includes
	  //edited cell)

	Errtype ensureSheetSize(int width,int height); //ensures that the sheet is at least the given size;
	                                               //useful for safe querying
};
