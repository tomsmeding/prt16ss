#include "model.h"
#include "util.h"
#include "conversion.h"
#include <stdexcept>

class Cell{
	set<CellAddress> revdeps; //reverse dependencies: cells that depend on this one
	const CellAddress addr; //address of this cell in sheet

public:
	Cell(CellAddress addr);
	virtual ~Cell();

	//returns a newly made cell with this value, and its dependencies
	//addr is its location in the sheet
	static pair<Cell*,vector<CellAddress>> cellFromString(string s,CellAddress addr,const CellArray &cells);

	//returns dependencies of the cell after change, or Nothing if no valid parse
	virtual Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells) = 0;

	virtual string getDisplayString() const = 0;
	virtual string getEditString() const = 0;

	//updates the cell, using possibly changed values of its dependencies
	virtual void update(const CellArray &cells) = 0;

	bool addReverseDependency(CellAddress addr); //false indicates already present
	void addReverseDependencies(const set<CellAddress> &addrs); //bulk version of addReverseDependency
	bool removeReverseDependency(CellAddress addr); //false indicates not present

	vector<CellAddress> getDependencies() const; //returns list of dependencies for this cell
	const set<CellAddress>& getReverseDependencies() const; //returns set of cells dependent on this cell
};

template <typename T>
class CellBasic : public Cell{
	T value;

public:
	//Cell interface:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);

	//Own interface:
	void setFromValue(T newval);
};

class CellFormula : public Cell{
public:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);
};

class CellError : public Cell{
	string errString;
	string editString;

public:
	//Cell interface:
	using Cell::Cell;

	Maybe<vector<CellAddress>> setEditString(string s,const CellArray &cells);
	string getDisplayString() const;
	string getEditString() const;

	void update(const CellArray &cells);

	//Own interface:
	void setErrorString(string s);
};



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
	Cell *cell=cells[addr.row][addr.column];
	Cell *newcell=Cell::cellFromString(repr,addr,cells).first;
	newcell->addReverseDependencies(cell->getReverseDependencies());
	delete cell;
	cells[addr.row][addr.column]=newcell;
	bool circularrefs;
	set<CellAddress> changed=recursiveUpdate(addr,&circularrefs);
	if(!circularrefs){
		return changed;
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



Cell::Cell(CellAddress addr)
	:addr(addr){}

Cell::~Cell(){}

pair<Cell*,vector<CellAddress>> Cell::cellFromString(string s,CellAddress addr,const CellArray &){
	Maybe<int> intval=convertstring<int>(s);
	if(intval.isJust()){
		CellBasic<int> *cell=new CellBasic<int>(addr);
		cell->setFromValue(intval.fromJust());
		return make_pair(cell,vector<CellAddress>());
	}
	Maybe<double> doubleval=convertstring<double>(s);
	if(doubleval.isJust()){
		CellBasic<double> *cell=new CellBasic<double>(addr);
		cell->setFromValue(doubleval.fromJust());
		return make_pair(cell,vector<CellAddress>());
	}
	/*if(s.size()&&s[0]=='='){
		//formula
		//on parse error, make cellerror
	}*/
	CellBasic<string> *cell=new CellBasic<string>(addr);
	cell->setFromValue(s);
	return make_pair(cell,vector<CellAddress>());
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
Maybe<vector<CellAddress>> CellBasic<T>::setEditString(string s,const CellArray&){
	s=trim(s);
	Maybe<T> mnewval=convertstring<T>(s);
	if(mnewval.isNothing())return Nothing();
	value=mnewval.fromJust();
	return vector<CellAddress>();
}

template <typename T>
void CellBasic<T>::setFromValue(T newval){
	value=newval;
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



Maybe<vector<CellAddress>> CellError::setEditString(string,const CellArray &){
	return Nothing();
}

string CellError::getDisplayString() const {
	return "ERR";
}

string CellError::getEditString() const {
	return "";
}

void CellError::update(const CellArray &){}

void CellError::setErrorString(string s){
	errString=s;
}
