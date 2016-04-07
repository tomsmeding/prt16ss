#include "cell.h"
#include "spreadsheet.h"
#include "util.h"
#include <fstream>
#include <vector>
#include <stdexcept>

unsigned int CellArray::width() const noexcept {
	return cells.size()==0?0:cells[0].size();
}

unsigned int CellArray::height() const noexcept {
	return cells.size();
}

Cell& CellArray::operator[](CellAddress addr) noexcept {
	return cells[addr.row][addr.column];
}

const Cell& CellArray::operator[](CellAddress addr) const noexcept {
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

CellArray::RangeWrapper CellArray::range(CellRange r) const noexcept {
	return RangeWrapper(*this,r);
}


CellArray::RangeWrapper::RangeWrapper(const CellArray &cells,CellRange range) noexcept
	:cells(&cells),range(range){}

CellArray::const_iterator CellArray::RangeWrapper::begin() const noexcept {
	return CellArrayIt(*cells,range);
}

CellArray::const_iterator CellArray::RangeWrapper::end() const noexcept {
	return CellArrayIt::endit();
}



CellArrayIt::CellArrayIt() noexcept
	:cells(nullptr),begin(0,0),end(0,0),cursor(0,0),isend(true){}

CellArrayIt::CellArrayIt(const CellArray &cells,CellRange r) noexcept
	:cells(&cells),begin(r.from),end(r.to),cursor(begin),isend(false){}

CellArrayIt CellArrayIt::endit() noexcept {
	return CellArrayIt();
}

bool CellArrayIt::operator==(const CellArrayIt &other) const noexcept {
	//if either is an end iterator, the rest of the data doesn't matter
	if(isend||other.isend)return isend==other.isend;

	return cells==other.cells&& //else just compare
	       begin==other.begin&&
	       end==other.end&&
	       cursor==other.cursor;
}

bool CellArrayIt::operator!=(const CellArrayIt &other) const noexcept {
	return !operator==(other);
}

const Cell& CellArrayIt::operator*() const {
	if(isend)throw logic_error("Dereference on end iterator (CellArrayIt)");
	return (*cells)[cursor];
}

const Cell* CellArrayIt::operator->() const {
	if(isend)throw logic_error("Dereference on end iterator (CellArrayIt)");
	return &(*cells)[cursor];
}

CellArrayIt& CellArrayIt::operator++() noexcept {
	if(isend)return *this;
	cursor.column++;
	if(cursor.column>end.column||cursor.column>=cells->width()){
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

unsigned int Spreadsheet::getWidth() const noexcept {
	return cells.width();
}

unsigned int Spreadsheet::getHeight() const noexcept {
	return cells.height();
}

bool Spreadsheet::inBounds(CellAddress addr) const noexcept {
	return addr.row<getHeight()&&addr.column<getWidth();
}


/*
Serialisation file format:
Every number is stored as an unsigned 32-bit int, in little-endian order.
Every string stored is prefixed with its length.
The file starts with the width and height of the sheet. Then the number of
revdepsOutside, followed by them, in pairs.
Finally all the cells, in row-major order.
*/

bool Spreadsheet::saveToDisk(string fname) const {
	ofstream out(fname);
	if(out.fail())return false;
	unsigned int x,y,w=getWidth(),h=getHeight();
	writeUInt32LE(out,w);
	writeUInt32LE(out,h);
	writeUInt32LE(out,revdepsOutside.size());
	for(const pair<CellAddress,set<CellAddress>> &p : revdepsOutside){
		p.first.serialise(out);
		writeUInt32LE(out,p.second.size());
		for(const CellAddress &addr : p.second){
			addr.serialise(out);
		}
	}
	string text;
	for(y=0;y<h;y++)for(x=0;x<w;x++){
		cells[CellAddress(y,x)].serialise(out);
		if(out.fail()){
			out.close();
			return false;
		}
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
	unsigned int nrdo=readUInt32LE(in);
	if(in.fail())return false;
	revdepsOutside.reserve(nrdo);
	for(x=0;x<nrdo;x++){
		CellAddress a=CellAddress::deserialise(in);
		int n=readUInt32LE(in);
		if(in.fail())return false;
		set<CellAddress> targets;
		for(int i=0;i<n;i++){
			targets.insert(CellAddress::deserialise(in));
		}
		revdepsOutside.emplace(a,targets);
	}
	string text;
	for(y=0;y<h;y++)for(x=0;x<w;x++){
		cells[CellAddress(y,x)].deserialise(in);
		if(in.fail())return false;
	}
	for(y=0;y<h;y++)for(x=0;x<w;x++){
		recursiveUpdate(CellAddress(y,x),nullptr,true);
	}
	in.close();
	return true;
}


Maybe<string> Spreadsheet::getCellDisplayString(CellAddress addr) noexcept {
	if(!inBounds(addr))return Nothing();
	return cells[addr].getDisplayString();
}

Maybe<string> Spreadsheet::getCellEditString(CellAddress addr) noexcept {
	if(!inBounds(addr))return Nothing();
	return cells[addr].getEditString();
}

bool Spreadsheet::checkCircularDependencies(CellAddress addr) noexcept {
	set<CellAddress> seen;
	return checkCircularDependencies(addr,seen);
}

bool Spreadsheet::checkCircularDependencies(CellAddress addr,set<CellAddress> &seen) noexcept {
	pair<set<CellAddress>::iterator,bool> insertret=seen.insert(addr);
	if(!insertret.second){ //already existed
		return true;
	}
	for(const CellAddress &revdepaddr : cells[addr].getReverseDependencies()){
		if(checkCircularDependencies(revdepaddr,seen))return true;
	}
	seen.erase(insertret.first);
	return false;
}

set<CellAddress> Spreadsheet::recursiveUpdate(CellAddress addr,
                                              bool *circularrefs,
                                              bool updatefirst) noexcept {
	if(circularrefs){
		if(checkCircularDependencies(addr)){
			*circularrefs=true;
			return set<CellAddress>();
		} else {
			*circularrefs=false;
		}
	}
	Cell &cell=cells[addr];
	if(updatefirst)cell.update(cells);
	set<CellAddress> seen;
	seen.insert(addr);
	set<CellAddress> revdeps=cell.getReverseDependencies();
	while(revdeps.size()){
		set<CellAddress> newrevdeps;
		for(CellAddress revdepaddr : revdeps){
			if(!seen.insert(revdepaddr).second){
				continue;
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

set<CellAddress> Spreadsheet::propagateError(CellAddress addr) noexcept {
	Cell &cell=cells[addr];
	const string &errString=cell.getDisplayString().substr(4); //strip "ERR:"
	set<CellAddress> seen;
	seen.insert(addr);
	set<CellAddress> revdeps=cell.getReverseDependencies();
	while(revdeps.size()){
		set<CellAddress> newrevdeps;
		for(CellAddress revdepaddr : revdeps){
			if(!seen.insert(revdepaddr).second){
				continue;
			}
			Cell &revdepcell=cells[revdepaddr];
			revdepcell.setError(errString);
			const set<CellAddress> d=revdepcell.getReverseDependencies();
			newrevdeps.insert(d.begin(),d.end());
		}
		revdeps=move(newrevdeps);
	}
	return seen;
}

void Spreadsheet::attachRevdeps(const vector<CellAddress> &depaddrs,CellAddress dest) noexcept {
	for(const CellAddress &depaddr : depaddrs){
		if(inBounds(depaddr)){
			cells[depaddr].addReverseDependency(dest);
		} else {
			auto it=revdepsOutside.find(depaddr);
			if(it==revdepsOutside.end()){
				revdepsOutside.emplace(depaddr,set<CellAddress>{dest});
			} else {
				it->second.insert(dest);
			}
		}
	}
}

void Spreadsheet::detachRevdeps(const vector<CellAddress> &depaddrs,CellAddress dest) noexcept {
	for(const CellAddress &depaddr : depaddrs){
		if(inBounds(depaddr)){
			cells[depaddr].removeReverseDependency(dest);
		} else {
			auto it=revdepsOutside.find(depaddr);
			if(it!=revdepsOutside.end()){
				auto vit=it->second.find(dest);
				if(vit!=it->second.end()){
					it->second.erase(vit);
				}
			}
		}
	}
}

Maybe<set<CellAddress>> Spreadsheet::changeCellValue(CellAddress addr,string repr) noexcept {
	if(!inBounds(addr))return Nothing();
	Cell &cell=cells[addr];
	detachRevdeps(cell.getDependencies(),addr);
	cell.setEditString(repr);
	const vector<CellAddress> newcelldeps=cell.getDependencies();
	for(const CellAddress &depaddr : newcelldeps){
		if(depaddr==addr){
			cell.setError("Self-circular reference");
			return recursiveUpdate(addr,nullptr,false);
		}
	}
	attachRevdeps(newcelldeps,addr);
	bool circularrefs;
	set<CellAddress> changed=recursiveUpdate(addr,&circularrefs,true);
	if(!circularrefs){
		return changed;
	}
	cell.setError("Circular reference chain");
	return propagateError(addr);
}

void Spreadsheet::ensureSheetSize(unsigned int width,unsigned int height){
	cells.ensureSize(width,height);
	vector<CellAddress> toerase;
	for(const pair<CellAddress,set<CellAddress>> &p : revdepsOutside){
		if(!inBounds(p.first))continue;
		cells[p.first].addReverseDependencies(p.second);
		toerase.push_back(p.first);
	}
	for(const CellAddress &addr : toerase){
		revdepsOutside.erase(revdepsOutside.find(addr));
	}
}
