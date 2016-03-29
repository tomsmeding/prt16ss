#pragma once

#include "celladdress.h"
#include "maybe.h"
#include "spreadsheet.h"
#include <vector>
#include <string>
#include <set>
#include <utility>

using namespace std;

class CellValue;

class Cell{
	CellValue *value;
	set<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress addr; //address of this cell in sheet

	Cell(CellValue *value,CellAddress addr);

public:
	Cell(CellAddress addr);
	Cell(string editString,CellAddress addr); //doesn't update() yet!
	~Cell();

	static Cell makeErrorCell(string errString,string editString,CellAddress addr);

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
};

class CellValue{
public:
	virtual ~CellValue();

	//returns a newly made cell with this value
	//addr is its location in the sheet;
	//not updated yet, do that with update()
	static CellValue* cellValueFromString(string s);

	virtual string getDisplayString() const = 0;
	virtual string getEditString() const = 0;

	//updates the cell, using possibly changed values of its dependencies
	//returns true if the cell must be regenerated with cellValueFromString
	virtual bool update(const CellArray &cells) = 0;

	virtual vector<CellAddress> getDependencies() const = 0; //returns list of dependencies for this cell
};

template <typename T>
class CellValueBasic : public CellValue{
	T value;

public:
	CellValueBasic(T value);

	string getDisplayString() const;
	string getEditString() const;

	bool update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;
};

class CellValueFormula : public CellValue{
	double doubleval;
	string stringval;
	bool isstring;
	string editString;
	vector<CellAddress> parsed;

	CellValueFormula() = default;

public:
	//returns Nothing() on parse error
	//not update()'d yet!
	static Maybe<CellValueFormula*> parseAndCreateFormula(string s);

	string getDisplayString() const;
	string getEditString() const;

	bool update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;
};

class CellValueError : public CellValue{
	string errString;
	string editString;

public:
	CellValueError(const string &errString,const string &editString);

	string getDisplayString() const;
	string getEditString() const;

	bool update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;
};
