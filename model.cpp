#include "model.h"
#include "util.h"
#include <stdexcept>

class Cell{
	vector<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress addr; //address of this cell in sheet

public:
	Cell(CellAddress addr);
	virtual ~Cell();

	//returns a newly made cell with this value, and its dependencies
	//addr is its location in the sheet
	static pair<Cell*,vector<CellAddress>> cellFromString(string s,CellAddress addr,const CellArray &cells);

	//returns dependencies of the cell after change, or Nothing if no valid parse
	virtual Maybe<vector<CellAddress>> setFromString(string s,const CellArray &cells) = 0;

	virtual string getDisplayString() const = 0;
	virtual string getEditString() const = 0;
};

template <typename T>
class CellBasic : public Cell{
	T value;

public:
	//Cell interface:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setFromString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	//Own interface:
	void setFromValue(T newval);
};

class CellFormula : public Cell{
public:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setFromString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;
};

class CellError : public Cell{
	string errString;
	string editString;

public:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setFromString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;
};

class CellEmpty : public Cell{
public:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setFromString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;
};


Spreadsheet::~Spreadsheet(){
	for(vector<Cell*> &row : cells){
		for(Cell *cell : row){
			delete cell;
		}
	}
}

void Spreadsheet::ensureSheetSize(unsigned int width,unsigned int height){
	for(size_t y=0;y<cells.size();y++){
		while(cells[y].size()<width){
			cells[y].push_back(new CellEmpty(CellAddress(y,cells[y].size())));
		}
	}
	while(cells.size()<height){
		cells.emplace_back();
		cells.back().reserve(width);
		for(size_t x=0;x<width;x++){
			cells.back().push_back(new CellEmpty(CellAddress(cells.size()-1,x)));
		}
	}
}


template <typename T>
Maybe<T> convertbasic(string s);

template <>
Maybe<int> convertbasic<int>(string s){
	s=trimright(s);
	size_t endpos;
	int intval=stoi(s,&endpos,0);
	if(endpos!=s.size())return Nothing();
	return intval;
}

template <>
Maybe<double> convertbasic<double>(string s){
	s=trimright(s);
	size_t endpos;
	double doubleval=stod(s,&endpos);
	if(endpos!=s.size())return Nothing();
	return doubleval;
}

template <>
Maybe<string> convertbasic<string>(string s){
	return s;
}


Cell::Cell(CellAddress addr)
	:addr(addr){}

Cell::~Cell(){}

pair<Cell*,vector<CellAddress>> Cell::cellFromString(string s,CellAddress addr,const CellArray &){
	Maybe<int> intval=convertbasic<int>(s);
	if(intval.isJust()){
		CellBasic<int> *cell=new CellBasic<int>(addr);
		cell->setFromValue(intval.fromJust());
		return make_pair(cell,vector<CellAddress>());
	}
	CellError *cell=new CellError(addr);
	return make_pair(cell,vector<CellAddress>());
}


template <typename T>
Maybe<vector<CellAddress>> CellBasic<T>::setFromString(string s,const CellArray&){
	s=trim(s);
	Maybe<T> mnewval=convertbasic<T>(s);
	if(mnewval.isNothing())return Nothing();
	value=mnewval.fromJust();
	return vector<CellAddress>();
}

template <typename T>
void CellBasic<T>::setFromValue(T newval){
	value=newval;
}

template <typename T>
string CellBasic<T>::getDisplayString() const {
	return to_string(value);
}

template <>
string CellBasic<string>::getDisplayString() const {
	return value;
}

template <typename T>
string CellBasic<T>::getEditString() const {
	return getDisplayString();
}


Maybe<vector<CellAddress>> CellError::setFromString(string,const CellArray &){
	return Nothing();
}

string CellError::getDisplayString() const {
	return "ERR";
}
string CellError::getEditString() const {
	return "";
}


Maybe<vector<CellAddress>> CellEmpty::setFromString(string,const CellArray &){
	return Nothing();
}

string CellEmpty::getDisplayString() const {
	return "";
}
string CellEmpty::getEditString() const {
	return "";
}
