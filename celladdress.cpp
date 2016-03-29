#include "celladdress.h"
#include "util.h"

using namespace std;

CellAddress::CellAddress(unsigned int row,unsigned int column)
	:row(row),column(column){}

Maybe<CellAddress> CellAddress::fromRepresentation(string repr){
	const int sz=repr.size();
	int i;
	int row=0,column=0;
	for(i=0;i<sz;i++){
		if(repr[i]<'A'||repr[i]>'Z')break;
		column=26*column+repr[i]-'A'+1;
	}
	if(i==0||i==sz){ //other things before letters, or no digits
		return Nothing();
	}
	column--;
	for(;i<sz;i++){
		if(repr[i]<'0'||repr[i]>'9'){ //other things after the digits
			return Nothing();
		}
		row=10*row+repr[i]-'0';
	}
	row--;
	return CellAddress(row,column);
}

string CellAddress::toRepresentation() const {
	string rev;
	unsigned int r=column+1;
	while(r){
		switch(r%26){
			case 0:
				rev+='Z';
				r=(r-26)/26;
				break;
			default:
				rev+=(char)('A'+r%26-1);
				r/=26;
				break;
		}
	}
	return string(rev.crbegin(),rev.crend())+to_string(row+1);
}

CellAddress CellAddress::deserialise(istream &in){
	unsigned int row=readUInt32LE(in);
	unsigned int column=readUInt32LE(in);
	return CellAddress(row,column);
}

void CellAddress::serialise(ostream &os) const {
	writeUInt32LE(os,row);
	writeUInt32LE(os,column);
}


bool operator==(const CellAddress &a,const CellAddress &b){
	return a.row==b.row&&a.column==b.column;
}


namespace std{
	bool less<CellAddress>::operator()(const CellAddress &a,const CellAddress &b){
		return a.row<b.row||(a.row==b.row&&a.column<b.column);
	}
}
