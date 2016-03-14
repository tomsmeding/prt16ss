#pragma once
#include "celladdress.h"
#include "maybe.h"
#include "spreadsheet.h"
#include <vector>
#include <string>
#include <set>
#include <utility>

using namespace std;

class Cell{
	set<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress addr; //address of this cell in sheet

public:
	Cell(CellAddress addr);
	virtual ~Cell();

	//returns a newly made cell with this value, and its dependencies
	//addr is its location in the sheet
	static pair<Cell*,vector<CellAddress>> cellFromString(string s,CellAddress addr,const CellArray &cells);

	const set<CellAddress>& getReverseDependencies() const; //returns set of cells dependent on this cell

	bool addReverseDependency(CellAddress addr); //false indicates already present
	void addReverseDependencies(const set<CellAddress> &addrs); //bulk version of addReverseDependency
	bool removeReverseDependency(CellAddress addr); //false indicates not present


	//returns dependencies of the cell after change, or Nothing if no valid parse
	virtual Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells) = 0;

	virtual string getDisplayString() const = 0;
	virtual string getEditString() const = 0;

	//updates the cell, using possibly changed values of its dependencies
	virtual void update(const CellArray &cells) = 0;

	virtual vector<CellAddress> getDependencies() const = 0; //returns list of dependencies for this cell
};

template <typename T>
class CellBasic : public Cell{
	T value;

public:
	//Cell interface:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;

	//Own interface:
	void setFromValue(T newval);
};

class CellFormula : public Cell{
	double doubleval;
	string stringval;
	bool isstring;
	string editString;

public:
	CellFormula(CellAddress addr);

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;
};

class CellError : public Cell{
	string errString;
	string editString;

public:
	//Cell interface:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;

	//Own interface:
	void setErrorString(string s);
};