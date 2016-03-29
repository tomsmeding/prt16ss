#pragma once

#include "maybe.h"
#include <string>

using namespace std;

class CellAddress{
public:
	unsigned int row,column; //left-top is (0,0)!
	// . . . .
	// . . . .
	// . # . .  <- that # is at (1,2) in documentation, and on row 2, column 1, and has representation "B3".
	// . . . .

	CellAddress(unsigned int row,unsigned int column);

	static Maybe<CellAddress> fromRepresentation(string repr); //converts something like "A1" to a CellAddress
	string toRepresentation() const; //returns a string representation of this

	static CellAddress deserialise(istream &in); //deserialises from stream into a new CellAddress
	void serialise(ostream &os) const;
};

bool operator==(const CellAddress &a,const CellAddress &b);

namespace std{
	template <>
	struct less<CellAddress>{
		bool operator()(const CellAddress &a,const CellAddress &b);
	};
}
