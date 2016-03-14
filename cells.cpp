#include "spreadsheet.h"
#include "cells.h"
#include "celladdress.h"
#include "conversion.h"
#include "maybe.h"
#include <vector>
#include <string>
#include <utility>

using namespace std;


Cell::Cell(CellAddress addr)
	:addr(addr){}

Cell::~Cell(){}

pair<Cell*,vector<CellAddress>> Cell::cellFromString(string s,CellAddress addr,const CellArray &cells){
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
	if(s.size()&&s[0]=='='){
		CellFormula *cell=new CellFormula(addr);
		Maybe<vector<CellAddress>> deps=cell->setEditString(s,cells);
		if(deps.isNothing()){
			delete cell;
			CellError *cell=new CellError(addr);
			cell->setErrorString("Invalid formula");
			cell->setEditString(s,cells);
			return make_pair(cell,vector<CellAddress>());
		}
		return make_pair(cell,deps.fromJust());
	}
	CellBasic<string> *cell=new CellBasic<string>(addr);
	cell->setFromValue(s);
	return make_pair(cell,vector<CellAddress>());
}

bool Cell::addReverseDependency(CellAddress addr){
	return revdeps.insert(addr).second;
}

void Cell::addReverseDependencies(const set<CellAddress> &addrs){
	for(const CellAddress &addr : addrs){
		revdeps.insert(addr);
	}
}

bool Cell::removeReverseDependency(CellAddress addr){
	auto it=revdeps.find(addr);
	if(it==revdeps.end())return false;
	revdeps.erase(it);
	return true;
}

const set<CellAddress>& Cell::getReverseDependencies() const {
	return revdeps;
}



template <typename T>
Maybe<vector<CellAddress>> CellBasic<T>::setEditString(string s,const CellArray&){
	s=trim(s);
	Maybe<T> mnewval=convertstring<T>(s);
	if(mnewval.isNothing())return Nothing();
	value=mnewval.fromJust();
	return vector<CellAddress>();
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

template <typename T>
void CellBasic<T>::update(const CellArray &){}

template <typename T>
vector<CellAddress> CellBasic<T>::getDependencies() const {
	return vector<CellAddress>();
}

template <typename T>
void CellBasic<T>::setFromValue(T newval){
	value=newval;
}



CellFormula::CellFormula(CellAddress addr)
	:Cell(addr),isstring(true){}

Maybe<vector<CellAddress>> CellFormula::setEditString(string,const CellArray &){
	//STUB
}

string CellFormula::getDisplayString() const {
	if(isstring)return stringval;
	else return to_string(doubleval);
}

string CellFormula::getEditString() const {
	return editString;
}

void CellFormula::update(const CellArray &){
	//STUB
}

vector<CellAddress> CellFormula::getDependencies() const {
	//STUB
}



Maybe<vector<CellAddress>> CellError::setEditString(string,const CellArray &){
	return Nothing();
}

string CellError::getDisplayString() const {
	return "ERR";
}

string CellError::getEditString() const {
	return editString;
}

void CellError::update(const CellArray &){}

vector<CellAddress> CellError::getDependencies() const {
	return vector<CellAddress>();
}

void CellError::setErrorString(string s){
	errString=s;
}
