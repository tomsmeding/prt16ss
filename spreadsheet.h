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
	void resize(unsigned int w,unsigned int h);

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

	//returns all cells updated, including given cell (only updated if updatefirst);
	//if circular references and circularrefs!=nullptr, sets that to true, else to false
	//if circularrefs==nullptr, ignores a circular reference and continues updating the rest
	//Assumes addr in bounds!
	set<CellAddress> recursiveUpdate(CellAddress addr,bool *circularrefs,bool updatefirst);

	//checks whether the chain of dependencies starting from addr contains a cycle
	//the second method should not be used directly; the first calls the second
	bool checkCircularDependencies(CellAddress addr);
	bool checkCircularDependencies(CellAddress addr,set<CellAddress> &seen);

public:
	Spreadsheet(unsigned int width,unsigned int height);

	//functions for saving and loading to/from files;
	//return whether successful
	bool saveToDisk(string fname) const;
	bool loadFromDisk(string fname);

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
