#include "spreadsheet.h"
#include "cells.h"
#include <vector>
#include <cassert>

Spreadsheet::~Spreadsheet(){
	for(const vector<Cell*> &row : cells){
		for(Cell *cell : row){
			delete cell;
		}
	}
}

unsigned int Spreadsheet::getWidth() const {
	return getHeight()?cells[0].size():0;
}

unsigned int Spreadsheet::getHeight() const {
	return cells.size();
}

bool Spreadsheet::inBounds(CellAddress addr) const {
	return addr.row<getHeight()&&addr.column<getWidth();
}

Maybe<string> Spreadsheet::getCellDisplayString(CellAddress addr) const {
	if(!inBounds(addr))return Nothing();
	return cells[addr.row][addr.column]->getDisplayString();
}

Maybe<string> Spreadsheet::getCellEditString(CellAddress addr) const {
	if(!inBounds(addr))return Nothing();
	return cells[addr.row][addr.column]->getEditString();
}

//returns all cells updated, including given cell (also updated);
//if circular references and circularrefs!=nullptr, sets that to true, else to false
//if circularrefs==nullptr, ignores a circular reference and continues updating the rest
//Assumes addr in bounds!
set<CellAddress> Spreadsheet::recursiveUpdate(CellAddress addr,bool *circularrefs){
	if(circularrefs)*circularrefs=false;
	Cell *cell=cells[addr.row][addr.column];
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
			Cell *revdepcell=cells[revdepaddr.row][revdepaddr.column];
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
	Cell *oldcell=cells[addr.row][addr.column];
	for(const CellAddress &depaddr : oldcell->getDependencies()){
		cells[depaddr.row][depaddr.column]->removeReverseDependency(addr);
	}
	pair<Cell*,vector<CellAddress>> newcellpair=Cell::cellFromString(repr,addr,cells);
	Cell *newcell=newcellpair.first;
	newcell->addReverseDependencies(oldcell->getReverseDependencies());
	for(const CellAddress &depaddr : newcell->getDependencies()){
		cells[depaddr.row][depaddr.column]->addReverseDependency(addr);
	}
	delete oldcell;
	cells[addr.row][addr.column]=newcell;
	bool circularrefs;
	set<CellAddress> changed=recursiveUpdate(addr,&circularrefs);
	if(!circularrefs){
		return changed;
	}
	for(const CellAddress &depaddr : newcell->getDependencies()){
		cells[depaddr.row][depaddr.column]->removeReverseDependency(addr);
	}
	CellError *errorcell=new CellError(addr);
	errorcell->setErrorString("Circular reference chain");
	errorcell->setEditString(newcell->getEditString(),cells);
	errorcell->addReverseDependencies(newcell->getReverseDependencies());
	delete newcell;
	cells[addr.row][addr.column]=errorcell;
	return recursiveUpdate(addr,nullptr);
}

void Spreadsheet::ensureSheetSize(unsigned int width,unsigned int height){
	assert(width!=-1U&&height!=-1U); //protection against error values
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
