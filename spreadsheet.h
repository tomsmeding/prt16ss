#pragma once

#include "types.h"
#include "celladdress.h"
#include <vector>
#include <set>
#include <string>

using namespace std;

class Cell;

class CellArrayIt;

class CellArray{
	vector<vector<Cell>> cells;

public:
	using iterator = CellArrayIt;

	unsigned int width() const;
	unsigned int height() const;

	Cell& operator[](CellAddress addr);
	const Cell& operator[](CellAddress addr) const;
	Cell& at(CellAddress addr);

	void ensureSize(unsigned int w,unsigned int h);

	/*CellArray::iterator begin();
	CellArray::iterator end();*/
	CellArray::iterator range(CellAddress a,CellAddress b);
};

class CellArrayIt : public iterator<input_iterator_tag,Cell*>{
	CellArray *cells;
	CellAddress begin,end,cursor;
	bool isend;

	CellArrayIt(); //end constructor
public:
	CellArrayIt(CellArray &cells,CellAddress begin,CellAddress end);

	static CellArrayIt endit();

	bool operator==(const CellArrayIt &other) const;
	bool operator!=(const CellArrayIt &other) const;
	Cell operator*() const;
	Cell* operator->() const;
	CellArrayIt& operator++();
};

class Spreadsheet{
	CellArray cells;

	unsigned int getWidth() const; //return dimensions of `cells`
	unsigned int getHeight() const;
	bool inBounds(CellAddress addr) const;

	//returns all cells updated, including given cell (also updated);
	//if circular references and circularrefs!=nullptr, sets that to true, else to false
	//if circularrefs==nullptr, ignores a circular reference and continues updating the rest
	//Assumes addr in bounds!
	set<CellAddress> recursiveUpdate(CellAddress addr,bool *circularrefs=nullptr);

public:
	Spreadsheet() = default;
	Spreadsheet(unsigned int width,unsigned int height);

	Errtype saveToDisk(string fname) const; //asks the model to save itself to the specified file
	Errtype loadFromDisk(string fname); //asks the model to load itself from the specified file

	Maybe<string> getCellDisplayString(CellAddress addr); //gets string for that cell for display in the sheet;
	                                                      //(Nothing if out of bounds)
	Maybe<string> getCellEditString(CellAddress addr); //gets string containing the raw cell data (for editing);
	                                                   //(Nothing if out of bounds)

	Maybe<set<CellAddress>> changeCellValue(CellAddress addr,string repr);
	  //changes the raw cell data of a cell, returns list of cells changed in sheet (includes
	  //edited cell) (Nothing if out of bounds)

	void ensureSheetSize(unsigned int width,unsigned int height); //ensures that the sheet is at least the given size;
	                                                              //useful for safe querying
};
