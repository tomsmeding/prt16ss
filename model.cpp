#include "model.h"
#include "util.h"
#include "conversion.h"
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
			cells[y].push_back(new CellBasic<string>(CellAddress(y,cells[y].size())));
		}
	}
	while(cells.size()<height){
		cells.emplace_back();
		cells.back().reserve(width);
		for(size_t x=0;x<width;x++){
			cells.back().push_back(new CellBasic<string>(CellAddress(cells.size()-1,x)));
		}
	}
}



Cell::Cell(CellAddress addr)
	:addr(addr){}

Cell::~Cell(){}

pair<Cell*,vector<CellAddress>> Cell::cellFromString(string s,CellAddress addr,const CellArray &){
	Maybe<int> intval=convertstring<int>(s);
	if(intval.isJust()){
		CellBasic<int> *cell=new CellBasic<int>(addr);
		cell->setFromValue(intval.fromJust());
		return make_pair(cell,vector<CellAddress>());
	}
	Maybe<double> doubleval=convertstring<double>(s);
	if(doubleval.isJust()){
		CellBasic<double> *cell=new CellBasic<double>(addr);
		cell->setFromValue(doubleval.fromJust());
		return make_pair(cell,vector<CellAddress>());
	}
	/*if(s.size()&&s[0]=='='){
		//formula
		//on parse error, make cellerror
	}*/
	CellBasic<string> *cell=new CellBasic<string>(addr);
	cell->setFromValue(s);
	return make_pair(cell,vector<CellAddress>());
}



template <typename T>
Maybe<vector<CellAddress>> CellBasic<T>::setFromString(string s,const CellArray&){
	s=trim(s);
	Maybe<T> mnewval=convertstring<T>(s);
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
