#pragma once

#include "celladdress.h"
#include <vector>
#include <set>
#include <unordered_map>
#include <string>

using namespace std;

/*
The main spreadsheet include file, for use in the other components.
This defines CellArray, the container for the actual array of cells in use in
Spreadsheet, and Spreadsheet itself.

CellArray is a 2D store of Cell's. Cell access is via CellAddress'es; a
const_iterator type is provided via range() using a CellRange.

Spreadsheet is a high-level spreadsheet object, usable without direct knowledge
of the actual implementation of the values; notably including formulas, which
are transparently handled.
*/

class Cell;

class CellArrayIt;

class CellArray{
	vector<vector<Cell>> cells;

public:
	using const_iterator = CellArrayIt;

	class RangeWrapper{
		const CellArray *cells;
		CellRange range;

	public:
		RangeWrapper(const CellArray &cells,CellRange range) noexcept;

		CellArray::const_iterator begin() const noexcept;
		CellArray::const_iterator end() const noexcept;
	};

	unsigned int width() const noexcept;
	unsigned int height() const noexcept;

	Cell& operator[](CellAddress addr) noexcept; //unsafe element access
	const Cell& operator[](CellAddress addr) const noexcept;
	Cell& at(CellAddress addr); //throws on out-of-bounds using vector::at

	void ensureSize(unsigned int w,unsigned int h); //only resizes up if needed
	void resize(unsigned int w,unsigned int h); //can forcibly resize down

	RangeWrapper range(CellRange r) const noexcept; //iterator provider
};

class CellArrayIt : public iterator<input_iterator_tag,Cell*>{
	const CellArray *cells;
	CellAddress begin,end,cursor;
	bool isend;

	CellArrayIt() noexcept; //end constructor
public:
	CellArrayIt(const CellArray &cells,CellRange r) noexcept;

	static CellArrayIt endit() noexcept; //returns the special end iterator

	bool operator==(const CellArrayIt &other) const noexcept;
	bool operator!=(const CellArrayIt &other) const noexcept;
	const Cell& operator*() const;
	const Cell* operator->() const;
	CellArrayIt& operator++() noexcept;
};

class Spreadsheet{
	CellArray cells;

	//reverse dependencies outside of allocated area
	//key is cell that is depended on by the value
	unordered_map<CellAddress,set<CellAddress>> revdepsOutside;

	unsigned int getWidth() const noexcept; //return dimensions of `cells`
	unsigned int getHeight() const noexcept;
	bool inBounds(CellAddress addr) const noexcept; //whether addr is in bounds

	//returns all cells updated, including given cell (only updated if
	//updatefirst)
	//if circular references and circularrefs!=nullptr, sets that to true, else
	//to false;  if circularrefs==nullptr, ignores a circular reference and
	//continues updating the rest
	//Assumes addr in bounds!
	set<CellAddress> recursiveUpdate(CellAddress addr,
		                             bool *circularrefs,
		                             bool updatefirst) noexcept;

	//checks whether the dep chain starting from addr contains a cycle
	//the second method should not be used directly; the first calls the second
	bool checkCircularDependencies(CellAddress addr) noexcept;
	bool checkCircularDependencies(CellAddress addr,set<CellAddress> &seen) noexcept;

public:
	Spreadsheet(unsigned int width,unsigned int height);

	//functions for saving and loading to/from files;
	//return whether successful
	bool saveToDisk(string fname) const;
	bool loadFromDisk(string fname);

	//gets display string for that cell (Nothing if out of bounds)
	Maybe<string> getCellDisplayString(CellAddress addr) noexcept;
	//gets the raw cell data (for editing) (Nothing if out of bounds)
	Maybe<string> getCellEditString(CellAddress addr) noexcept;

	//changes the raw cell data of a cell, returns list of cells changed in
	//sheet (includes edited cell); (Nothing if out of bounds)
	Maybe<set<CellAddress>> changeCellValue(CellAddress addr,string repr) noexcept;

	//ensures that the sheet is at least the given size;
	//useful for safe querying
	void ensureSheetSize(unsigned int width,unsigned int height);
};
