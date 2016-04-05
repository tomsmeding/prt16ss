#pragma once

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

/*
Some utility functions
*/

//trim whitespace from the right
string trimright(const string &s) noexcept;

//centre-align the specified string in a window of that width
string centreString(const string &s,int wid) noexcept;

//binary IO for streams
void writeUInt32LE(ostream &os,uint32_t v);
uint32_t readUInt32LE(istream &is);

//zero-based column to a letter-based column label
string columnLabel(int col) noexcept;
