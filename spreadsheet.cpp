#include "spreadsheet.h"
#include "cells.h"
#include "util.h"
#include <fstream>
#include <vector>
#include <stdexcept>

unsigned int CellArray::width() const {
	return cells.size()==0?0:cells[0].size();
}

unsigned int CellArray::height() const {
	return cells.size();
}

Cell& CellArray::operator[](CellAddress addr){
	return cells[addr.row][addr.column];
}

const Cell& CellArray::operator[](CellAddress addr) const {
	return cells[addr.row][addr.column];
}

Cell& CellArray::at(CellAddress addr){
	return cells.at(addr.row).at(addr.column);
}

void CellArray::ensureSize(unsigned int w,unsigned int h){
	resize(max(w,width()),max(h,height()));
}

void CellArray::resize(unsigned int w,unsigned int h){
	if(w==-1U||h==-1U){ //protection against error values
		throw out_of_range("-1 dimension in CellArray::ensureSize");
	}
	if(w<width()){
		for(size_t y=0;y<height();y++){
			for(size_t x=width()-1;x>=w;x--){
				cells[y].pop_back();
			}
		}
	} else if(w>width()){
		for(size_t y=0;y<height();y++){
			cells[y].reserve(w);
			for(size_t x=cells[y].size();x<w;x++){
				cells[y].emplace_back(CellAddress(y,cells[y].size()));
			}
		}
	}
	if(h<height()){
		cells.erase(cells.begin()+h,cells.end());
	} else {
		while(h>height()){
			cells.emplace_back();
			cells.back().reserve(w);
			for(size_t x=0;x<w;x++){
				cells.back().emplace_back(CellAddress(cells.size()-1,x));
			}
		}
	}
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

Cell CellArrayIt::operator*() const {
	if(isend)throw logic_error("Dereference on end iterator (CellArrayIt)");
	return (*cells)[cursor];
}

Cell* CellArrayIt::operator->() const {
	if(isend)throw logic_error("Dereference on end iterator (CellArrayIt)");
	return &(*cells)[cursor];
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


/*
Serialisation file format:
Every number is stored as an unsigned 32-bit int, in little-endian order.
Every string stored is prefixed with its length.
The file starts with the width and height of the sheet. Then follow all the
cells, in row-major order.
*/

bool Spreadsheet::saveToDisk(string fname) const {
	ofstream out(fname);
	if(out.fail())return false;
	unsigned int x,y,w=getWidth(),h=getHeight();
	writeUInt32LE(out,w);
	writeUInt32LE(out,h);
	string text;
	for(y=0;y<h;y++)for(x=0;x<w;x++){
		cells[CellAddress(y,x)].serialise(out);
		if(out.fail()){
			out.close();
			return false;
		}
	}
	if(out.fail()){
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool Spreadsheet::loadFromDisk(string fname){
	ifstream in(fname);
	if(in.fail())return false;
	unsigned int x,y,w,h;
	w=readUInt32LE(in);
	h=readUInt32LE(in);
	if(in.fail())return false;
	cells.resize(w,h);
	string text;
	for(y=0;y<h;y++)for(x=0;x<w;x++){
		cells[CellAddress(y,x)].deserialise(in);
		if(in.fail())return false;
	}
	if(in.fail()){
		in.close();
		return false;
	}
	in.close();
	return true;
}


Maybe<string> Spreadsheet::getCellDisplayString(CellAddress addr) {
	if(!inBounds(addr))return Nothing();
	return cells[addr].getDisplayString();
}

Maybe<string> Spreadsheet::getCellEditString(CellAddress addr) {
	if(!inBounds(addr))return Nothing();
	return cells[addr].getEditString();
}

set<CellAddress> Spreadsheet::recursiveUpdate(CellAddress addr,bool *circularrefs,bool updatefirst){
	if(circularrefs)*circularrefs=false;
	Cell &cell=cells[addr];
	if(updatefirst)cell.update(cells);
	set<CellAddress> seen;
	seen.insert(addr);
	set<CellAddress> revdeps=cell.getReverseDependencies();
	while(revdeps.size()){
		set<CellAddress> newrevdeps;
		for(CellAddress revdepaddr : revdeps){
			if(!seen.insert(revdepaddr).second){
				if(circularrefs)*circularrefs=true;
				return seen;
			}
			Cell &revdepcell=cells[revdepaddr];
			revdepcell.update(cells);
			const set<CellAddress> d=revdepcell.getReverseDependencies();
			newrevdeps.insert(d.begin(),d.end());
		}
		revdeps=move(newrevdeps);
	}
	return seen;
}

Maybe<set<CellAddress>> Spreadsheet::changeCellValue(CellAddress addr,string repr){
	if(!inBounds(addr))return Nothing();
	Cell &cell=cells[addr];
	for(const CellAddress &depaddr : cell.getDependencies()){
		cells[depaddr].removeReverseDependency(addr);
	}
	cell.setEditString(repr);
	for(const CellAddress &depaddr : cell.getDependencies()){
		cells[depaddr].addReverseDependency(addr);
	}
	bool circularrefs;
	set<CellAddress> changed=recursiveUpdate(addr,&circularrefs,true);
	if(!circularrefs){
		return changed;
	}
	cell.setError("Circular reference chain");
	return recursiveUpdate(addr,nullptr,false);
}

void Spreadsheet::ensureSheetSize(unsigned int width,unsigned int height){
	cells.ensureSize(width,height);
}
