#include "model.h"
#include "util.h"
#include <stdexcept>

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
