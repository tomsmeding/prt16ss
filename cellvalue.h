#pragma once

#include "maybe.h"
#include "either.h"
#include <string>
#include <vector>

using namespace std;

/*
A number of classes, all subclass of CellValue, to be used in Cell. Basic values
are covered in CellValueBasic, formulas in CellValueFormula, and error values
in CellValueError. The last distinction is necessary, because an error cell also
needs to store its original edit string, alongside the error string. This dual
string storage is unique to the error cell.
*/

class CellArray;
class CellAddress;

class CellValue{
public:
	virtual ~CellValue();

	//returns a newly made cell with this value
	//addr is its location in the sheet;
	//not updated yet, do that with update()
	static CellValue* cellValueFromString(string s) noexcept;

	virtual string getDisplayString() const = 0;
	virtual string getEditString() const = 0;

	//updates the cell, using possibly changed values of its dependencies
	//returns true if the cell must be regenerated with cellValueFromString
	virtual bool update(const CellArray &cells) = 0;

	//returns list of dependencies for this cell
	virtual vector<CellAddress> getDependencies() const = 0;
};

template <typename T>
class CellValueBasic : public CellValue{
	T value;

public:
	CellValueBasic(T value) noexcept
		:value(value){}

	string getDisplayString() const noexcept;
	string getEditString() const noexcept;

	bool update(const CellArray &cells) noexcept;

	vector<CellAddress> getDependencies() const noexcept;
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
	static Either<string,CellValueFormula*> parseAndCreateFormula(string s) noexcept;

	string getDisplayString() const noexcept;
	string getEditString() const noexcept;

	bool update(const CellArray &cells) noexcept;

	vector<CellAddress> getDependencies() const noexcept;
};

class CellValueError : public CellValue{
	string errString;
	string editString;

public:
	CellValueError(const string &errString,const string &editString) noexcept;

	string getDisplayString() const noexcept;
	string getEditString() const noexcept;
	string getErrorString() const noexcept;

	bool update(const CellArray &cells) noexcept;

	vector<CellAddress> getDependencies() const noexcept;
};
