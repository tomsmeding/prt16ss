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

Cell* Cell::cellFromString(string s,CellAddress addr){
	Maybe<int> intval=convertstring<int>(s);
	if(intval.isJust()){
		CellBasic<int> *cell=new CellBasic<int>(addr);
		cell->setFromValue(intval.fromJust());
		return cell;
	}
	Maybe<double> doubleval=convertstring<double>(s);
	if(doubleval.isJust()){
		CellBasic<double> *cell=new CellBasic<double>(addr);
		cell->setFromValue(doubleval.fromJust());
		return cell;
	}
	if(s.size()&&s[0]=='='){
		CellFormula *cell=new CellFormula(addr);
		if(!cell->setEditString(s)){
			delete cell;
			CellError *cell=new CellError(addr,s);
			cell->setErrorString("Invalid formula");
		}
		return cell;
	}
	CellBasic<string> *cell=new CellBasic<string>(addr);
	cell->setFromValue(s);
	return cell;
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
bool CellBasic<T>::setEditString(string s){
	s=trim(s);
	Maybe<T> mnewval=convertstring<T>(s);
	if(mnewval.isNothing())return false;
	value=mnewval.fromJust();
	return true;
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

bool CellFormula::setEditString(string s){ //STUB
	size_t cursor=0,idx,sz=s.size();
	parsed.clear();
	while(cursor<sz){
		idx=s.find(' ',cursor);
		if(idx==string::npos)idx=sz;
		Maybe<CellAddress> mca=CellAddress::fromRepresentation(s.substr(cursor,idx-cursor));
		if(mca.isNothing()){
			parsed.clear();
			return false;
		}
		for(cursor=idx;cursor<sz;cursor++){
			if(s[cursor]!=' ')break;
		}
	}
	return true;
}

string CellFormula::getDisplayString() const {
	if(isstring)return stringval;
	else return to_string(doubleval);
}

string CellFormula::getEditString() const {
	return editString;
}

void CellFormula::update(const CellArray &cells){ //STUB
	isstring=true;
	stringval.clear();
	size_t sz=parsed.size();
	for(size_t i=0;i<sz;i++){
		if(i!=0)stringval+=' ';
		if(parsed[i].row>=cells.height()||parsed[i].column>=cells.width()){
			//FIX THIS
			stringval+="???";
		}
	}
}

vector<CellAddress> CellFormula::getDependencies() const { //STUB
	return parsed;
}



CellError::CellError(CellAddress addr,const string &editString)
	:Cell(addr),editString(editString){}

bool CellError::setEditString(string){
	return false;
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
