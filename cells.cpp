#include "spreadsheet.h"
#include "cells.h"
#include "celladdress.h"
#include "conversion.h"
#include "maybe.h"
#include <vector>
#include <string>
#include <utility>

using namespace std;


Cell::Cell(CellValue *value,CellAddress addr)
	:value(value),addr(addr){}

Cell::Cell(CellAddress addr)
	:value(new CellValueBasic<string>("")),addr(addr){}

Cell::Cell(string editString,CellAddress addr)
	:value(nullptr),addr(addr){
	setEditString(editString);
}

Cell::~Cell(){}

Cell Cell::makeErrorCell(string errString,string editString,CellAddress addr){
	return Cell(new CellValueError(errString,editString),addr);
}

void Cell::setError(string errString){
	CellValue *newvalue;
	if(value){
		newvalue=new CellValueError(errString,value->getEditString());
		delete value;
	} else {
		newvalue=new CellValueError(errString,"");
	}
	value=newvalue;
}

const set<CellAddress>& Cell::getReverseDependencies() const {
	return revdeps;
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

void Cell::setEditString(string s){
	if(value)delete value;
	value=CellValue::cellValueFromString(s);
}

string Cell::getDisplayString() const {
	return value->getDisplayString();
}

string Cell::getEditString() const {
	return value->getEditString();
}

void Cell::update(const CellArray &cells){
	if(value->update(cells)){
		CellValue *newvalue=CellValue::cellValueFromString(value->getEditString());
		delete value;
		value=newvalue;
		value->update(cells);
	}
}

vector<CellAddress> Cell::getDependencies() const {
	return value->getDependencies();
}



CellValue::~CellValue(){}


CellValue* CellValue::cellValueFromString(string s){
	Maybe<int> intval=convertstring<int>(s);
	if(intval.isJust()){
		return new CellValueBasic<int>(intval.fromJust());
	}
	Maybe<double> doubleval=convertstring<double>(s);
	if(doubleval.isJust()){
		return new CellValueBasic<double>(doubleval.fromJust());
	}
	if(s.size()&&s[0]=='='){
		Maybe<CellValueFormula*> mcv=CellValueFormula::parseAndCreateFormula(s);
		if(mcv.isNothing()){
			return new CellValueError("Invalid formula",s);
		}
		return mcv.fromJust();
	}
	return new CellValueBasic<string>(s);
}



template <typename T>
CellValueBasic<T>::CellValueBasic(T value)
	:value(value){}

template <typename T>
string CellValueBasic<T>::getDisplayString() const {
	return to_string(value);
}

template <>
string CellValueBasic<string>::getDisplayString() const {
	return value;
}

template <typename T>
string CellValueBasic<T>::getEditString() const {
	return getDisplayString();
}

template <typename T>
bool CellValueBasic<T>::update(const CellArray &){
	return false;
}

template <typename T>
vector<CellAddress> CellValueBasic<T>::getDependencies() const {
	return vector<CellAddress>();
}



Maybe<CellValueFormula*> CellValueFormula::parseAndCreateFormula(string s){ //STUB
	CellValueFormula *cv=new CellValueFormula;
	size_t cursor=1,idx,sz=s.size();
	while(cursor<sz){
		idx=s.find(' ',cursor);
		if(idx==string::npos)idx=sz;
		Maybe<CellAddress> mca=CellAddress::fromRepresentation(s.substr(cursor,idx-cursor));
		if(mca.isNothing()){
			return Nothing();
		}
		cv->parsed.push_back(mca.fromJust());
		for(cursor=idx;cursor<sz;cursor++){
			if(s[cursor]!=' ')break;
		}
	}
	cv->editString=s;
	return cv;
}

string CellValueFormula::getDisplayString() const {
	if(isstring)return stringval;
	else return to_string(doubleval);
}

string CellValueFormula::getEditString() const {
	return editString;
}

bool CellValueFormula::update(const CellArray &cells){ //STUB
	isstring=true;
	stringval.clear();
	size_t sz=parsed.size();
	for(size_t i=0;i<sz;i++){
		if(i!=0)stringval+=' ';
		if(parsed[i].row>=cells.height()||parsed[i].column>=cells.width()){
			//FIX THIS
			stringval+="???";
		} else {
			stringval+=cells[parsed[i]].getDisplayString();
		}
	}
	return false;
}

vector<CellAddress> CellValueFormula::getDependencies() const { //STUB
	return parsed;
}



CellValueError::CellValueError(const string &errString,const string &editString)
	:errString(errString),editString(editString){}

string CellValueError::getDisplayString() const {
	return "ERR:"+errString;
}

string CellValueError::getEditString() const {
	return editString;
}

bool CellValueError::update(const CellArray &){
	return true;
}

vector<CellAddress> CellValueError::getDependencies() const {
	return vector<CellAddress>();
}
