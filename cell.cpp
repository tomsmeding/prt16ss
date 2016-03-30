#include "spreadsheet.h"
#include "cell.h"
#include "cellvalue.h"
#include "celladdress.h"
#include "conversion.h"
#include "maybe.h"
#include "util.h"
#include <vector>
#include <string>
#include <utility>

using namespace std;


Cell::Cell(CellValue *value,CellAddress address)
	:value(value),address(address){}

Cell::Cell(CellAddress address)
	:value(new CellValueBasic<string>("")),address(address){}

Cell::Cell(string editString,CellAddress address)
	:value(nullptr),address(address){
	setEditString(editString);
}

Cell::~Cell(){}

Cell Cell::makeErrorCell(string errString,string editString,CellAddress address){
	return Cell(new CellValueError(errString,editString),address);
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

/*
Serialisation format:
First the number of reverse dependencies, then a list of them;
followed by the edit string.
*/
void Cell::serialise(ostream &os) const {
	writeUInt32LE(os,revdeps.size());
	for(const CellAddress &revdepaddr : revdeps){
		revdepaddr.serialise(os);
	}
	const string &s=value->getEditString();
	writeUInt32LE(os,s.size());
	os<<s;
}

void Cell::deserialise(istream &in){
	unsigned int nrevdeps=readUInt32LE(in);
	if(in.fail())return; //random allocation prevention
	revdeps.clear();
	unsigned int i;
	//cerr<<"Cell::des("<<address.toRepresentation()<<"): revdeps:";
	for(i=0;i<nrevdeps;i++){
		CellAddress ca=CellAddress::deserialise(in);
		//cerr<<' '<<ca.toRepresentation();
		revdeps.insert(ca);
		// revdeps.insert(CellAddress::deserialise(in));
	}
	//cerr<<"; edit: ";
	unsigned int len=readUInt32LE(in);
	if(in.fail())return;
	string s;
	s.resize(len);
	in.read(&s.front(),len);
	//cerr<<s<<endl;
	setEditString(s);
}
