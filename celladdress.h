#pragma once

#include "maybe.h"
#include <string>

using namespace std;

/*
CellAddress is an address of a cell in the spreadsheet. Structurally equivalent
to std::pair, but with some methods applicable to spreadsheet addresses.
The row and column attributes are public on purpose: the class is simply a
container of the two values with some convenience methods. Therefore, getters
and setters are not needed.

CellRange is just a pair of CellAddresses, again with some special methods.
In the data, no care is taken to ensure that e.g. the x-coordinate of the first
address is <= that of the second; that is, however, taken care of by the
representation conversion functions and CellArray::range().
*/

class CellAddress{
public:
	unsigned int row,column; //left-top is (0,0)
	//   A B C D
	// 1 . . . .
	// 2 . . . .
	// 3 . # . .  <- that # is at (x,y)=(1,2) in memory, and on row 2, column 1,
	// 4 . . . .     and has representation "B3".

	CellAddress(unsigned int row,unsigned int column) noexcept;

	//converts something like "A1" to a CellAddress
	static Maybe<CellAddress> fromRepresentation(string repr) noexcept;

	//returns a string representation of this
	string toRepresentation() const noexcept;

	//deserialises from stream into a new CellAddress
	static CellAddress deserialise(istream &in);

	//serialises this address to the stream
	void serialise(ostream &os) const;
};

bool operator==(const CellAddress &a,const CellAddress &b) noexcept;

namespace std{
	template <>
	struct less<CellAddress>{ //for set<CellAddress>
		bool operator()(const CellAddress &a,const CellAddress &b) const noexcept;
	};

	template <>
	struct hash<CellAddress>{ //for unordered_map<CellAddress> 
		size_t operator()(const CellAddress &a) const noexcept;
	};
}


class CellRange{
public:
	CellAddress from,to;

	CellRange(CellAddress from,CellAddress to) noexcept;

	//converts from something like "A1:C3"
	static Maybe<CellRange> fromRepresentation(string repr) noexcept;

	//returns a string representation of this
	string toRepresentation() const noexcept;

	//number of cells spanned
	unsigned int size() const noexcept;
};

bool operator==(const CellRange &a,const CellRange &b) noexcept;
