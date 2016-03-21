#include "spreadsheet.h"
#include "cells.h"
#include <vector>
#include <stdexcept>

CellArray::~CellArray(){
	for(const vector<Cell*> &row : cells){
		for(Cell *cell : row){
			delete cell;
		}
	}
}

unsigned int CellArray::width() const {
	return cells.size()==0?0:cells[0].size();
}

unsigned int CellArray::height() const {
	return cells.size();
}

Cell*& CellArray::operator[](CellAddress addr){
	return cells[addr.row][addr.column];
}

Cell*& CellArray::at(CellAddress addr){
	return cells.at(addr.row).at(addr.column);
}

void CellArray::ensureSize(unsigned int w,unsigned int h){
	if(w==-1U||h==-1U){ //protection against error values
		throw out_of_range("-1 dimension in CellArray::ensureSize");
	}
	for(size_t y=0;y<height();y++){
		while(cells[y].size()<w){
			cells[y].push_back(new CellBasic<string>(CellAddress(y,cells[y].size())));
		}
	}
	while(height()<h){
		cells.emplace_back();
		cells.back().reserve(w);
		for(size_t x=0;x<w;x++){
			cells.back().push_back(new CellBasic<string>(CellAddress(cells.size()-1,x)));
		}
	}
}

CellArray::iterator CellArray::begin(){
	return CellArrayIt(*this,CellAddress(0,0),CellAddress(height()-1,width()-1));
}

CellArray::iterator CellArray::end(){
	return CellArrayIt::endit();
}

CellArray::iterator CellArray::range(CellAddress a,CellAddress b){
	return CellArrayIt(*this,a,b);
}



CellArrayIt::CellArrayIt()
	:cells(nullptr),begin(0,0),end(0,0),cursor(0,0),isend(true){}

CellArrayIt::CellArrayIt(CellArray &cells,CellAddress begin,CellAddress end)
	:cells(&cells),begin(begin),end(end),cursor(begin),isend(false){}

CellArrayIt CellArrayIt::endit(){
	return CellArrayIt();
}

bool CellArrayIt::operator==(const CellArrayIt &other) const {
	if(isend||other.isend)return isend==other.isend;
	return cells==other.cells&&
	       begin==other.begin&&
	       end==other.end&&
	       cursor==other.cursor;
}

bool CellArrayIt::operator!=(const CellArrayIt &other) const {
	return !operator==(other);
}

Cell* CellArrayIt::operator*() const {
	if(isend)return nullptr;
	else return (*cells)[cursor];
}

Cell** CellArrayIt::operator->() const {
	if(isend)return nullptr;
	else return &(*cells)[cursor];
}

CellArrayIt& CellArrayIt::operator++(){
	cursor.column++;
	if(cursor.column>end.column){
		cursor.column=begin.column;
		cursor.row++;
		if(cursor.row>end.row){
			isend=true;
		}
	}
	return *this;
}



Spreadsheet::Spreadsheet(unsigned int width,unsigned int height){
	ensureSheetSize(width,height);
}

unsigned int Spreadsheet::getWidth() const {
	return cells.width();
}

unsigned int Spreadsheet::getHeight() const {
	return cells.height();
}

bool Spreadsheet::inBounds(CellAddress addr) const {
	return addr.row<getHeight()&&addr.column<getWidth();
}

Maybe<string> Spreadsheet::getCellDisplayString(CellAddress addr) {
	if(!inBounds(addr))return Nothing();
	return cells[addr]->getDisplayString();
}

Maybe<string> Spreadsheet::getCellEditString(CellAddress addr) {
	if(!inBounds(addr))return Nothing();
	return cells[addr]->getEditString();
}

//returns all cells updated, including given cell (also updated);
//if circular references and circularrefs!=nullptr, sets that to true, else to false
//if circularrefs==nullptr, ignores a circular reference and continues updating the rest
//Assumes addr in bounds!
set<CellAddress> Spreadsheet::recursiveUpdate(CellAddress addr,bool *circularrefs){
	if(circularrefs)*circularrefs=false;
	Cell *cell=cells[addr];
	cell->update(cells);
	set<CellAddress> seen;
	seen.insert(addr);
	set<CellAddress> revdeps=cell->getReverseDependencies();
	while(revdeps.size()){
		set<CellAddress> newrevdeps;
		for(CellAddress revdepaddr : revdeps){
			if(!seen.insert(revdepaddr).second&&circularrefs){
				*circularrefs=true;
				return seen;
			}
			Cell *revdepcell=cells[revdepaddr];
			revdepcell->update(cells);
			const set<CellAddress> d=revdepcell->getReverseDependencies();
			newrevdeps.insert(d.begin(),d.end());
		}
		revdeps=move(newrevdeps);
	}
	return seen;
}

Maybe<set<CellAddress>> Spreadsheet::changeCellValue(CellAddress addr,string repr){
	if(!inBounds(addr))return Nothing();
	Cell *oldcell=cells[addr];
	for(const CellAddress &depaddr : oldcell->getDependencies()){
		cells[depaddr]->removeReverseDependency(addr);
	}
	Cell *newcell=Cell::cellFromString(repr,addr);
	newcell->addReverseDependencies(oldcell->getReverseDependencies());
	for(const CellAddress &depaddr : newcell->getDependencies()){
		cells[depaddr]->addReverseDependency(addr);
	}
	delete oldcell;
	cells[addr]=newcell;
	bool circularrefs;
	set<CellAddress> changed=recursiveUpdate(addr,&circularrefs);
	if(!circularrefs){
		return changed;
	}
	for(const CellAddress &depaddr : newcell->getDependencies()){
		cells[depaddr]->removeReverseDependency(addr);
	}
	CellError *errorcell=new CellError(addr,newcell->getEditString());
	errorcell->setErrorString("Circular reference chain");
	errorcell->addReverseDependencies(newcell->getReverseDependencies());
	delete newcell;
	cells[addr]=errorcell;
	return recursiveUpdate(addr,nullptr);
}

void Spreadsheet::ensureSheetSize(unsigned int width,unsigned int height){
	cells.ensureSize(width,height);
}
