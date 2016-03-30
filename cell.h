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

class CellValue;

class Cell{
	CellValue *value;
	set<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress address; //address of this cell in sheet

	Cell(CellValue *value,CellAddress address);

public:
	Cell(CellAddress address);
	Cell(string editString,CellAddress address); //doesn't update() yet!
	~Cell();

	static Cell makeErrorCell(string errString,string editString,CellAddress address);

	void setError(string errString);

	const set<CellAddress>& getReverseDependencies() const; //returns set of cells dependent on this cell

	bool addReverseDependency(CellAddress addr); //false indicates already present
	void addReverseDependencies(const set<CellAddress> &addrs); //bulk version of addReverseDependency
	bool removeReverseDependency(CellAddress addr); //false indicates not present

	//doesn't update the cell's display string, do that with update()
	void setEditString(string s);

	string getDisplayString() const;
	string getEditString() const;

	//updates the cell, using possibly changed values of its dependencies
	void update(const CellArray &cells);

	vector<CellAddress> getDependencies() const; //returns list of dependencies for this cell

	void serialise(ostream &os) const; //serialises the cell to the stream
	void deserialise(istream &in); //deserialises the cell from the stream
};
