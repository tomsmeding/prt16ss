#include "cellvalue.h"
#include "celladdress.h"
#include "spreadsheet.h"
#include "conversion.h"
#include "cell.h"
#include "formula.h"

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
		Either<string,CellValueFormula*> mcv=CellValueFormula::parseAndCreateFormula(s);
		if(mcv.isLeft()){
			return new CellValueError("Invalid formula: "+mcv.fromLeft(),s);
		}
		return mcv.fromRight();
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



Either<string,CellValueFormula*> CellValueFormula::parseAndCreateFormula(string s){
	Either<string,Formula*> mparsed=Formula::parse(s.substr(1));
	if(mparsed.isLeft())return mparsed.fromLeft();
	CellValueFormula *cv=new CellValueFormula;
	cv->parsed=mparsed.fromRight();
	cv->editString=s;
	return cv;
}

string CellValueFormula::getDisplayString() const {
	return dispString;
}

string CellValueFormula::getEditString() const {
	return editString;
}

bool CellValueFormula::update(const CellArray &cells){
	dispString=parsed->evaluate(cells);
	return false;
}

vector<CellAddress> CellValueFormula::getDependencies() const {
	return parsed->getDependencies();
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
