#pragma once

#include "celladdress.h"
#include "maybe.h"
#include "spreadsheet.h"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <utility>

using namespace std;

/*
A cell in the spreadsheet; contains a CellValue (an instance from an abstract
base class) to keep its value. Supports management of reverse dependencies, and
serialisation.
*/

class CellArray;

class CellValue;

class Cell{
	CellValue *value;
	set<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress address; //address of this cell in sheet

	Cell(CellValue *value,CellAddress address) noexcept;

public:
	Cell(CellAddress address) noexcept;
	Cell(string editString,CellAddress address) noexcept; //doesn't update() yet!
	~Cell() noexcept;

	static Cell makeErrorCell(string errString,string editString,CellAddress address) noexcept;

	void setError(string errString) noexcept;

	//returns set of cells dependent on this cell
	const set<CellAddress>& getReverseDependencies() const noexcept;

	//false indicates already present
	bool addReverseDependency(CellAddress addr) noexcept;
	//bulk version of addReverseDependency
	void addReverseDependencies(const set<CellAddress> &addrs) noexcept;
	//false indicates not present
	bool removeReverseDependency(CellAddress addr) noexcept;

	//doesn't update the cell's display string, do that with update()
	void setEditString(string s) noexcept;

	string getDisplayString() const noexcept;
	string getEditString() const noexcept;

	//returns whether the cell contains an error value
	bool isErrorValue() const noexcept;

	//updates the cell, using possibly changed values of its dependencies
	void update(const CellArray &cells) noexcept;

	//returns list of dependencies for this cell
	vector<CellAddress> getDependencies() const noexcept;

	void serialise(ostream &os) const; //serialises the cell to the stream
	void deserialise(istream &in); //deserialises the cell from the stream
};
