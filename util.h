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

//binary IO for streams
void writeUInt32LE(ostream &os,uint32_t v);
uint32_t readUInt32LE(istream &is);
