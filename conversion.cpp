#include "conversion.h"
#include "util.h"
#include <stdexcept>

using namespace std;

template <>
Maybe<int> convertstring<int>(string s) noexcept {
	s=trimright(s);
	size_t endpos;
	int intval;
	try {
		intval=stoi(s,&endpos,0);
	} catch(invalid_argument e){
		return Nothing();
	} catch(out_of_range e){
		return Nothing();
	}
	if(endpos!=s.size())return Nothing();
	return intval;
}

template <>
Maybe<double> convertstring<double>(string s) noexcept {
	s=trimright(s);
	size_t endpos;
	double doubleval;
	try {
		doubleval=stod(s,&endpos);
	} catch(invalid_argument e){
		return Nothing();
	} catch(out_of_range e){
		return 1/0.0; //==inf
	}
	if(endpos!=s.size())return Nothing();
	return doubleval;
}

template <>
Maybe<string> convertstring<string>(string s) noexcept {
	return s;
}
