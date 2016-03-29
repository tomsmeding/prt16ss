#include "util.h"
#include <cctype>

string trim(const string &s){
	int sz=s.size();
	int left=0;
	while(left<sz&&isspace(s[left]))left++;
	if(left==sz)return "";
	int right=sz-1;
	while(isspace(s[right]))right--;
	return s.substr(left,right-left+1);
}

string trimleft(const string &s){
	int sz=s.size();
	int left=0;
	while(left<sz&&isspace(s[left]))left++;
	if(left==sz)return "";
	return s.substr(left);
}

string trimright(const string &s){
	int sz=s.size();
	int right=sz-1;
	while(right>=0&&isspace(s[right]))right--;
	if(right==-1)return "";
	return s.substr(0,right+1);
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
