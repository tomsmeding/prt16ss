#pragma once

#include "maybe.h"
#include "util.h"
#include <string>

using namespace std;

template <typename T>
Maybe<T> convertstring(string s);

template <>
Maybe<int> convertstring<int>(string s){
	s=trimright(s);
	size_t endpos;
	int intval=stoi(s,&endpos,0);
	if(endpos!=s.size())return Nothing();
	return intval;
}

template <>
Maybe<double> convertstring<double>(string s){
	s=trimright(s);
	size_t endpos;
	double doubleval=stod(s,&endpos);
	if(endpos!=s.size())return Nothing();
	return doubleval;
}

template <>
Maybe<string> convertstring<string>(string s){
	return s;
}
