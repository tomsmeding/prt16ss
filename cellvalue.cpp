#include "cell.h"
#include "cellvalue.h"
#include "celladdress.h"
#include "spreadsheet.h"
#include "conversion.h"
#include "formula.h"
#include <sstream>

using namespace std;

CellValue::~CellValue() noexcept {}

CellValue* CellValue::cellValueFromString(string s) noexcept {
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
string CellValueBasic<T>::getDisplayString() const noexcept {
	stringstream ss;
	ss<<value;
	return ss.str();
}

template <>
string CellValueBasic<string>::getDisplayString() const noexcept {
	return value;
}

template <typename T>
string CellValueBasic<T>::getEditString() const noexcept {
	return getDisplayString();
}

template <typename T>
bool CellValueBasic<T>::update(const CellArray &) noexcept {
	return false;
}

template <typename T>
vector<CellAddress> CellValueBasic<T>::getDependencies() const noexcept {
	return vector<CellAddress>();
}



Either<string,CellValueFormula*> CellValueFormula::parseAndCreateFormula(string s) noexcept {
	Either<string,Formula*> mparsed=Formula::parse(s.substr(1));
	if(mparsed.isLeft())return mparsed.fromLeft();
	CellValueFormula *cv=new CellValueFormula;
	cv->parsed=mparsed.fromRight();
	cv->editString=s;
	return cv;
}

string CellValueFormula::getDisplayString() const noexcept {
	return dispString;
}

string CellValueFormula::getEditString() const noexcept {
	return editString;
}

bool CellValueFormula::update(const CellArray &cells) noexcept {
	Maybe<string> res=parsed->evaluate(cells);
	if(res.isNothing()){
		dispString="FERR:Error in formula dependencies";
	} else dispString=res.fromJust();
	return false;
}

vector<CellAddress> CellValueFormula::getDependencies() const noexcept {
	return parsed->getDependencies();
}



CellValueError::CellValueError(const string &errString,const string &editString) noexcept
	:errString(errString),editString(editString){}

string CellValueError::getDisplayString() const noexcept {
	return "ERR:"+errString;
}

string CellValueError::getEditString() const noexcept {
	return editString;
}

string CellValueError::getErrorString() const noexcept {
	return errString;
}

bool CellValueError::update(const CellArray &) noexcept {
	return true;
}

vector<CellAddress> CellValueError::getDependencies() const noexcept {
	CellValue *cv=CellValue::cellValueFromString(editString);
	vector<CellAddress> deps=cv->getDependencies();
	delete cv;
	return deps;
}
