#pragma once

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

//trims whitespace on both sides
string trim(const string &s);

//trim whitespace on one side
string trimleft(const string &s);
string trimright(const string &s);

//binary IO for streams
void writeUInt32LE(ostream &os,uint32_t v);
uint32_t readUInt32LE(istream &is);
