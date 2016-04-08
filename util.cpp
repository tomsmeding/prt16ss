// Tom Smeding (s1685694), Tim Brouwer (s1663615), Ruben Turkenburg (s1659685)

#include "util.h"
#include <cctype>

string trimright(const string &s) noexcept {
	int sz=s.size();
	int right=sz-1;
	while(right>=0&&isspace(s[right]))right--;
	if(right==-1)return "";
	return s.substr(0,right+1);
}

string centreString(const string &s,int wid) noexcept {
	return string((wid-s.size())/2,' ')+s+string((wid-s.size()+1)/2,' ');
}

void writeUInt32LE(ostream &os,uint32_t v){
	unsigned char buf[4];
	int i;
	for(i=0;i<4;i++){
		buf[i]=v&0xff;
		v>>=8;
	}
	os.write((char*)buf,4);
}

uint32_t readUInt32LE(istream &is){
	unsigned char buf[4];
	is.read((char*)buf,4);
	int i,v=0;
	for(i=4-1;i>=0;i--){
		v=(v<<8)|buf[i];
	}
	return v;
}

string columnLabel(int col) noexcept {
	string rev;
	col++;
	while(col){
		switch(col%26){
			case 0:
				rev+='Z';
				col=(col-26)/26;
				break;
			default:
				rev+=(char)('A'+col%26-1);
				col/=26;
				break;
		}
	}
	return string(rev.crbegin(),rev.crend());
}
