#pragma once

#include "maybe.h"
#include "either.h"
#include <string>
#include <vector>

using namespace std;

class CellArray;
class CellAddress;

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
	CellValueBasic(T value)
		:value(value){}

	string getDisplayString() const;
	string getEditString() const;

	bool update(const CellArray &cells);

	vector<CellAddress> getDependencies() const;
};


class Formula;

class CellValueFormula : public CellValue{
	Formula *parsed;
	string dispString;
	string editString;

	CellValueFormula() = default;

public:
	//returns Nothing() on parse error
	//not update()'d yet!
	static Either<string,CellValueFormula*> parseAndCreateFormula(string s);

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
