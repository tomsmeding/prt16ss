#include "cellvalue.h"
#include "celladdress.h"
#include "spreadsheet.h"
#include "conversion.h"
#include "cell.h"

using namespace std;

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
