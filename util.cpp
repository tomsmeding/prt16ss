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
